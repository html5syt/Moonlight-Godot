using Godot;
using System;
using System.Runtime.InteropServices;
using FFmpeg.AutoGen; // 引用 NuGet 包

// 需要允许 unsafe 代码
public unsafe partial class MoonlightPlayer : Control
{
    [Export] public string HostAddress = "192.168.1.100";
    [Export] public TextureRect VideoDisplay; // 拖入场景中的 TextureRect

    var myGDScriptNode = (GodotObject)MoonlightStream.New();

    private MoonlightStream _streamer;
    private ImageTexture _videoTexture;
    private Image _imageBuffer;
    
    // FFmpeg 上下文
    private AVCodec* _codec;
    private AVCodecContext* _codecContext;
    private AVFrame* _decodedFrame;
    private AVPacket* _packet;
    
    // 软件缩放/格式转换上下文 (YUV -> RGBA)
    private SwsContext* _swsContext;
    private int _currentWidth = 0;
    private int _currentHeight = 0;
    private byte_ptrArray4 _dstData;
    private int_array4 _dstLinesize;

    public override void _Ready()
    {
        // 1. 初始化 FFmpeg 库路径
        ConfigureFFmpeg();

        // 2. 创建 GDExtension 节点
        _streamer = new MoonlightStream();
        _streamer.Name = "MoonlightStreamInternal";
        AddChild(_streamer);

        // 3. 配置 Moonlight
        _streamer.SetHostAddress(HostAddress);
        _streamer.SetResolution(1920, 1080);
        _streamer.SetFps(60);
        _streamer.SetVideoCodec(MoonlightStream.Codec.CodecH264); // 或 HEVC
        
        // 4. 连接信号
        // 注意：C++ 发出的信号参数是 (PackedByteArray data, int frameType)
        _streamer.Connect("video_packet_received", Callable.From<byte[], int>(OnVideoPacketReceived));
        _streamer.Connect("connection_started", Callable.From(OnConnectionStarted));
        
        // 初始化解码器
        InitDecoder(AVCodecID.AV_CODEC_ID_H264); // 如果用 HEVC 这里要改
    }

    private void ConfigureFFmpeg()
    {
        // 指定 dll 路径，根据实际存放位置修改
        ffmpeg.RootPath = ProjectSettings.GlobalizePath("res://ffmpeg/bin"); 
        GD.Print($"FFmpeg Path: {ffmpeg.RootPath}");
    }

    private void InitDecoder(AVCodecID codecId)
    {
        _codec = ffmpeg.avcodec_find_decoder(codecId);
        if (_codec == null) throw new Exception("Codec not found");

        _codecContext = ffmpeg.avcodec_alloc_context3(_codec);
        if (ffmpeg.avcodec_open2(_codecContext, _codec, null) < 0)
            throw new Exception("Could not open codec");

        _packet = ffmpeg.av_packet_alloc();
        _decodedFrame = ffmpeg.av_frame_alloc();
    }

    private void OnConnectionStarted()
    {
        GD.Print("C# Logic: Connection Started!");
    }

    // 信号回调：接收视频数据
    private void OnVideoPacketReceived(byte[] data, int frameType)
    {
        if (data.Length == 0) return;

        // 将 C# byte[] 数据送入 FFmpeg
        fixed (byte* pData = data)
        {
            // 填充 Packet
            // 注意：这里可能需要手动处理 H.264 Annex B start codes (00 00 00 01)
            // Moonlight (Limelight) 通常发送的是已经包含 start code 的 NALU，直接喂给 FFmpeg 即可。
            
            _packet->data = pData;
            _packet->size = data.Length;
            
            // 发送 Packet 到解码器
            int ret = ffmpeg.avcodec_send_packet(_codecContext, _packet);
            if (ret < 0)
            {
                GD.PrintErr($"Error sending packet: {ret}");
                return;
            }

            // 循环接收解码后的 Frame (一个 Packet 可能解出多个 Frame，虽然流媒体通常是一对一)
            while (true)
            {
                ret = ffmpeg.avcodec_receive_frame(_codecContext, _decodedFrame);
                if (ret == ffmpeg.AVERROR(ffmpeg.EAGAIN) || ret == ffmpeg.AVERROR_EOF)
                    break;
                if (ret < 0)
                {
                    GD.PrintErr("Error receiving frame");
                    break;
                }

                // 处理解码后的帧
                ProcessDecodedFrame(_decodedFrame);
            }
        }
    }

    private void ProcessDecodedFrame(AVFrame* frame)
    {
        int width = frame->width;
        int height = frame->height;

        // 如果分辨率变化或初次运行，初始化 SwsContext 和 Godot Texture
        if (_swsContext == null || width != _currentWidth || height != _currentHeight)
        {
            _currentWidth = width;
            _currentHeight = height;

            // 释放旧的
            if (_swsContext != null) ffmpeg.sws_freeContext(_swsContext);

            // 创建 YUV420P -> RGBA 转换上下文
            _swsContext = ffmpeg.sws_getContext(
                width, height, _codecContext->pix_fmt,
                width, height, AVPixelFormat.AV_PIX_FMT_RGBA,
                ffmpeg.SWS_FAST_BILINEAR, null, null, null
            );

            // 分配目标缓冲区 (RGBA = 4 bytes per pixel)
            int bufferSize = ffmpeg.av_image_get_buffer_size(AVPixelFormat.AV_PIX_FMT_RGBA, width, height, 1);
            // 这里为了简单演示，每次都 new，实际生产环境应该复用非托管内存或 byte[]
            // 但 Godot 的 Image.CreateFromData 需要 byte[]
            
            // 设置 dstLinesize
            ffmpeg.av_image_fill_arrays(ref _dstData, ref _dstLinesize, null, AVPixelFormat.AV_PIX_FMT_RGBA, width, height, 1);
        }

        // 准备 C# 数组接收 RGBA 数据
        byte[] rgbaBytes = new byte[width * height * 4];
        
        fixed (byte* pDst = rgbaBytes)
        {
            // 重新指向我们的托管数组
            _dstData.data[0] = pDst;
            _dstLinesize.data[0] = width * 4;

            // 执行 YUV -> RGBA 转换
            ffmpeg.sws_scale(
                _swsContext,
                frame->data, frame->linesize,
                0, height,
                _dstData.data, _dstLinesize
            );
        }

        // --- 回到 Godot 主线程更新 UI ---
        // Image 操作需要在主线程稍微安全些，虽然 SetData 是数据拷贝
        CallDeferred(nameof(UpdateTexture), rgbaBytes, width, height);
    }

    private void UpdateTexture(byte[] rgbaData, int width, int height)
    {
        if (_imageBuffer == null || _imageBuffer.GetWidth() != width || _imageBuffer.GetHeight() != height)
        {
            _imageBuffer = Image.CreateFromData(width, height, false, Image.Format.Rgba8, rgbaData);
            _videoTexture = ImageTexture.CreateFromImage(_imageBuffer);
            if (VideoDisplay != null) VideoDisplay.Texture = _videoTexture;
        }
        else
        {
            // 直接更新数据，避免创建新对象
            _imageBuffer.SetData(width, height, false, Image.Format.Rgba8, rgbaData);
            _videoTexture.Update(_imageBuffer);
        }
    }

    public void StartStream()
    {
        _streamer.StartConnection();
    }

    public void StopStream()
    {
        _streamer.StopConnection();
    }

    public override void _ExitTree()
    {
        // 清理 FFmpeg 资源
        if (_packet != null) ffmpeg.av_packet_free(ref _packet);
        if (_decodedFrame != null) ffmpeg.av_frame_free(ref _decodedFrame);
        if (_codecContext != null) ffmpeg.avcodec_free_context(ref _codecContext);
        if (_swsContext != null) ffmpeg.sws_freeContext(_swsContext);
        
        base._ExitTree();
    }
}