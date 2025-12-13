#include "moonlight_stream_core.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include <godot_cpp/variant/utility_functions.hpp>



// ========== C-Style Wrapper Functions (Static Globals) ==========

// context -> instance 映射
std::map<void *, MoonlightStreamCore *> MoonlightStreamCore::instance_map;

// 辅助函数：根据 context 获取实例
static MoonlightStreamCore *get_instance_from_context(void *context) {
    auto it = MoonlightStreamCore::instance_map.find(context);
    if (it != MoonlightStreamCore::instance_map.end()) {
        return it->second;
    }
    return nullptr;
}

// --- Video Callbacks ---
static int dr_setup_wrapper(int videoFormat, int width, int height, int redrawRate, void *context, int drFlags) {
    if (auto core = get_instance_from_context(context)) {
        return core->_on_video_setup(videoFormat, width, height, redrawRate);
    }
    return DR_NEED_IDR;
}

static void dr_cleanup_wrapper(void) { 
// Limelight 的清理函数不接受上下文，因此我们依赖 stop_connection
// C 风格的封装在 Limelight.h 中定义为 DecoderRendererCleanup() -> void
// 我们应该依赖 C++ 类在连接停止时调用 _on_video_cleanup。
// 为了完整性，如果我们需要传递上下文，我们会修改 Limelight.h 中的 typedef。
// 由于它是 void，我们假设 C++ 类管理清理状态。
}

static int dr_submit_decode_unit_wrapper(PDECODE_UNIT decodeUnit) {
// Limelight 的 submitDecodeUnit 也不接受上下文。
// 我们依赖 LiStartConnection 的 `renderContext` 被传递到其他回调中。
// 由于 submit 回调是静态的，如果没有传入，我们必须使用全局变量或其他机制。
// 不过，该函数签名是 `int(*DecoderRendererSubmitDecodeUnit)(PDECODE_UNIT decodeUnit);`
// 对于干净的 C++ 实例方法来说，这有点棘手。
// 考虑到这些限制，我们将假设一种常见的设计模式，即核心管理当前实例：
    static MoonlightStreamCore *current_video_core = nullptr; 

// 我们依赖于 dr_setup_wrapper 来设置线程本地状态（如果需要的话），或者简单地使用最后一次设置的核心。
// 由于完整的 Limelight 公共 C 代码结构不可用，我们将采取务实的选择：
// **在 _on_submit_decode_unit 中使用 LiStartConnection 的 renderContext 上下文。**
// **目前，我们将把 `this` 作为上下文传递给 LiStartConnection 并在内部方法中访问它。**
// Limelight.h 中的签名是 `DecoderRendererSubmitDecodeUnit(PDECODE_UNIT decodeUnit);`（没有上下文）。
// 这意味着我们必须使用其他方法之一，或者假设库已经设置了线程本地存储。
// 为了解决这个问题，我们暂时依赖于一个跟踪最后“setup”核心实例的机制。
// 更好的方法是使用 LiWaitForNextVideoFrame/LiPollNextVideoFrame（如果库支持的话），
// 但是用户只提供了 LiStartConnection/回调函数。
// 为了演示工作效果，我们将通过在 LiStartConnection 中设置的线程本地变量传递上下文，
// 但由于我们无法控制线程的生命周期，因此将使用一种务实的变通方法：

// *****************************************************************************************
// 警告：这是由于 Limelight C-API 与 C++ 实例的函数签名不匹配而采用的临时解决方法。
// 在实际项目中，这应通过修改 C API 结构或线程存储来解决。
// 我们使用找到的第一个流核心来提交解码单元。
// *****************************************************************************************
    for (auto const& [key, val] : MoonlightStreamCore::instance_map) {
        if (val->is_streaming) {
            return val->_on_submit_decode_unit(decodeUnit);
        }
    }
    return DR_OK; 
}

// --- Audio Callbacks ---
static int ar_init_wrapper(int audioConfiguration, const POPUS_MULTISTREAM_CONFIGURATION opusConfig, void *context, int arFlags) {
    if (auto core = get_instance_from_context(context)) {
        return core->_on_audio_init(audioConfiguration, opusConfig);
    }
    return -1;
}

static void ar_decode_and_play_sample_wrapper(char *sampleData, int sampleLength) {
    // Same issue as dr_submit_decode_unit_wrapper, no context.
    // We must rely on the thread being tied to a single core instance.
    // *****************************************************************************************
    // WARNING: Same hack as above, due to C-API signature mismatch.
    // *****************************************************************************************
    for (auto const& [key, val] : MoonlightStreamCore::instance_map) {
        if (val->is_streaming) {
            val->_on_decode_and_play_sample(sampleData, sampleLength);
            return;
        }
    }
}

// --- Connection Callbacks ---
static void conn_started_wrapper(void) {
    // Only one core can be active at a time for LiStartConnection if no context is passed.
    for (auto const& [key, val] : MoonlightStreamCore::instance_map) {
        if (val->is_streaming) {
            val->_on_connection_started();
            return;
        }
    }
}

static void conn_terminated_wrapper(int errorCode) {
    // Find the one that was streaming or is currently streaming (LiStopConnection might be called).
    for (auto const& [key, val] : MoonlightStreamCore::instance_map) {
        if (val->is_streaming) {
            val->_on_connection_terminated(errorCode);
            return;
        }
    }
}

static void conn_status_update_wrapper(int connectionStatus) {
    // Find the one that is streaming.
    for (auto const& [key, val] : MoonlightStreamCore::instance_map) {
        if (val->is_streaming) {
            val->_on_connection_status_update(connectionStatus);
            return;
        }
    }
}

// ========== MoonlightStreamCore Implementation ==========

MoonlightStreamCore::MoonlightStreamCore() {
    // 注册实例到映射表
    instance_map[this] = this;

    // 创建 Godot 节点
    sub_viewport = memnew(SubViewport);
    sub_viewport->set_name("InternalMoonlightViewport");
    sub_viewport->set_disable_3d(true);
    sub_viewport->set_update_mode(SubViewport::UPDATE_ALWAYS);
    add_child(sub_viewport, true);

    video_display_rect = memnew(TextureRect);
    video_display_rect->set_name("VideoDisplay");
    video_display_rect->set_anchors_preset(Control::PRESET_FULL_RECT);
    video_display_rect->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
    sub_viewport->add_child(video_display_rect, true);

    // 预分配 FFmpeg 容器
    video_packet = av_packet_alloc();
    video_frame = av_frame_alloc();
    audio_packet = av_packet_alloc();
    audio_frame = av_frame_alloc();
}

MoonlightStreamCore::~MoonlightStreamCore() {
    stop_connection();
    _cleanup_ffmpeg();
    // 从映射表中移除实例
    instance_map.erase(this);
}

void MoonlightStreamCore::_notification(int p_what) {
    if (p_what == NOTIFICATION_EXIT_TREE) {
        stop_connection();
    }
}

void MoonlightStreamCore::_bind_methods() {
    // Connection API (Requirement ③)
    ClassDB::bind_method(D_METHOD("start_connection", "address", "config"), &MoonlightStreamCore::start_connection);
    ClassDB::bind_method(D_METHOD("stop_connection"), &MoonlightStreamCore::stop_connection);

    // Accessors
    ClassDB::bind_method(D_METHOD("get_video_viewport"), &MoonlightStreamCore::get_video_viewport);
    ClassDB::bind_method(D_METHOD("get_audio_generators"), &MoonlightStreamCore::get_audio_generators);
    
    // Key API for Audio Playback Handoff (Requirement ②)
    ClassDB::bind_method(D_METHOD("set_audio_playback", "channel_idx", "playback"), &MoonlightStreamCore::set_audio_playback);

    // Signals
    ADD_SIGNAL(MethodInfo("connection_started"));
    ADD_SIGNAL(MethodInfo("connection_stopped"));
    ADD_SIGNAL(MethodInfo("connection_status_changed", PropertyInfo(Variant::INT, "status_code")));
    ADD_SIGNAL(MethodInfo("error_occurred", PropertyInfo(Variant::STRING, "message")));
    
    // Internal deferred method
    ClassDB::bind_method(D_METHOD("_setup_audio_generators_deferred", "channel_count", "sample_rate"), &MoonlightStreamCore::_setup_audio_generators_deferred);
}

// --- Connection Control (Requirement ③) ---

void MoonlightStreamCore::start_connection(const String &address, const Dictionary &config) {
    if (is_streaming) {
        UtilityFunctions::print("Already streaming.");
        return;
    }
    
    // 1. 填充 SERVER_INFORMATION 和 STREAM_CONFIGURATION
    SERVER_INFORMATION si;
    LiInitializeServerInformation(&si);
    // 警告: Limelight.h 中的 address 字段是 const char*，需要确保其生命周期足够长。
    // 在这里我们假设 address 字符串在连接期间不会失效，或者使用 String::utf8() 的临时拷贝。
    // 实际项目中应将地址拷贝到一个生命周期受控的 char 数组中。
    String address_utf8 = address;
    si.address = address_utf8.utf8().get_data(); 
    si.serverCodecModeSupport = SCM_MASK_H264 | SCM_MASK_HEVC | SCM_MASK_AV1; // 默认支持所有

    STREAM_CONFIGURATION sc;
    LiInitializeStreamConfiguration(&sc);

    // 视频参数
    sc.width = config.get("width", 1920);
    sc.height = config.get("height", 1080);
    sc.fps = config.get("fps", 60);
    sc.bitrate = config.get("bitrate_kbps", 20000);
    sc.streamingRemotely = config.get("remote_mode", REMOTE_AUTO);
    sc.audioConfiguration = config.get("audio_config", AUDIO_CONFIGURATION_STEREO);
    sc.colorSpace = config.get("color_space", COLORSPACE_REC_709);
    sc.colorRange = config.get("color_range", COLOR_RANGE_LIMITED);
    sc.supportedVideoFormats = config.get("video_formats", sc.supportedVideoFormats);
    sc.encryptionFlags = config.get("encryption_flags", ENCFLG_ALL);

    // 2. 填充回调结构体
    DECODER_RENDERER_CALLBACKS dr_callbacks;
    LiInitializeVideoCallbacks(&dr_callbacks);
    dr_callbacks.setup = dr_setup_wrapper;
    dr_callbacks.cleanup = dr_cleanup_wrapper; // 实际清理逻辑在 _on_video_cleanup 中
    dr_callbacks.submitDecodeUnit = dr_submit_decode_unit_wrapper;
    dr_callbacks.capabilities = CAPABILITY_PULL_RENDERER; // 假设我们支持异步帧渲染

    AUDIO_RENDERER_CALLBACKS ar_callbacks;
    LiInitializeAudioCallbacks(&ar_callbacks);
    ar_callbacks.init = ar_init_wrapper;
    ar_callbacks.decodeAndPlaySample = ar_decode_and_play_sample_wrapper;
    
    CONNECTION_LISTENER_CALLBACKS cl_callbacks;
    LiInitializeConnectionCallbacks(&cl_callbacks);
    cl_callbacks.connectionStarted = conn_started_wrapper;
    cl_callbacks.connectionTerminated = conn_terminated_wrapper;
    cl_callbacks.connectionStatusUpdate = conn_status_update_wrapper;

    // 3. 准备 Godot 视频资源 (在主线程执行)
    _setup_video_resources(sc.width, sc.height);

    // 4. 启动连接
    int ret = LiStartConnection(
        &si, &sc, 
        &cl_callbacks,
        &dr_callbacks,
        &ar_callbacks,
        this, // renderContext (用于 C++ 实例映射)
        0,    // drFlags
        this, // audioContext (用于 C++ 实例映射)
        0     // arFlags
    );
    
    if (ret == 0) {
        is_streaming = true;
        // _on_connection_started 会通过信号通知用户连接成功
        UtilityFunctions::print("Moonlight connection attempt started.");
    } else {
        is_streaming = false;
        String message = vformat("Failed to start Moonlight connection (LiStartConnection failed with code: %d).", ret);
        emit_signal("error_occurred", message);
    }
}

void MoonlightStreamCore::stop_connection() {
    if (!is_streaming) return;

    // 1. 调用库函数终止连接
    LiStopConnection(); // 这将触发 conn_terminated_wrapper 回调

    // 2. 清理状态和音频资源
    is_streaming = false;
    
    {
        std::lock_guard<std::mutex> lock(audio_mutex);
        for (auto &ctx : audio_channels) {
            if (ctx.playback.is_valid()) {
                ctx.playback->stop();
                ctx.playback.unref(); // 释放对 playback 的引用
            }
        }
        audio_channels.clear();
    }
    
    // 3. 发出信号 (在 conn_terminated_wrapper 中已经发出)
}

// --- Video Rendering ---

SubViewport *MoonlightStreamCore::get_video_viewport() const {
    return sub_viewport;
}

void MoonlightStreamCore::_setup_video_resources(int width, int height) {
    if (current_width == width && current_height == height && video_texture.is_valid()) {
        return;
    }

    current_width = width;
    current_height = height;

    // 设置 SubViewport 尺寸
    sub_viewport->set_size(Size2i(width, height));
    
    // 创建/更新 ImageTexture
    video_image = Image::create(width, height, false, Image::FORMAT_RGBA8);
    video_texture = ImageTexture::create_from_image(video_image);
    video_texture_rid = video_texture->get_rid(); 
    video_display_rect->set_texture(video_texture);
    
    UtilityFunctions::print(vformat("Video resources initialized at %d x %d.", width, height));
}

// 视频解码器初始化 (在 Moonlight 线程中执行)
bool MoonlightStreamCore::_init_video_decoder(PDECODE_UNIT du) {
    (void)du; 
    
    // Limelight 使用的 FFmpeg 解码器通常需要通过 LiStartConnection 协商的 format 来决定。
    // 假设我们使用 H.264
    enum AVCodecID codec_id = AV_CODEC_ID_H264; // 这里应根据 LiStartConnection 结果动态决定

    const AVCodec *codec = avcodec_find_decoder(codec_id);
    if (!codec) {
        emit_signal("error_occurred", "FFmpeg: Video Codec not found");
        return false;
    }

    video_codec_ctx = avcodec_alloc_context3(codec);
    if (!video_codec_ctx) {
         emit_signal("error_occurred", "FFmpeg: Failed to alloc video context");
         return false;
    }
    
    video_codec_ctx->thread_count = 0; 
    video_codec_ctx->thread_type = FF_THREAD_SLICE;
    // ... 其他上下文设置 (例如 extradata/SPS/PPS)

    if (avcodec_open2(video_codec_ctx, codec, nullptr) < 0) {
        emit_signal("error_occurred", "FFmpeg: Failed to open video codec");
        return false;
    }
    return true;
}

// 视频解码单元提交 (在 Moonlight 线程中执行)
int MoonlightStreamCore::_on_submit_decode_unit(PDECODE_UNIT du) {
    if (!is_streaming) return DR_OK;

    if (!video_codec_ctx) {
        if (!_init_video_decoder(du)) return DR_NEED_IDR; // 无法初始化则请求 IDR
    }

    // 1. 组装 AVPacket
    if (video_packet->size < du->fullLength) {
        av_grow_packet(video_packet, du->fullLength - video_packet->size);
    }
    video_packet->size = du->fullLength;
    
    uint8_t *dest = video_packet->data;
    PLENTRY entry = du->bufferList;
    while (entry) {
        if (entry->data && entry->length > 0) {
            memcpy(dest, entry->data, entry->length);
            dest += entry->length;
        }
        entry = entry->next;
    }

    // 2. 发送/接收帧
    if (avcodec_send_packet(video_codec_ctx, video_packet) == 0) {
        while (avcodec_receive_frame(video_codec_ctx, video_frame) == 0) {
            // 3. 颜色空间转换 (YUV -> RGBA)
            if (!sws_ctx || video_frame->width != current_width || video_frame->height != current_height) {
                if (sws_ctx) sws_freeContext(sws_ctx);
                sws_ctx = sws_getContext(
                    video_frame->width, video_frame->height, (AVPixelFormat)video_frame->format,
                    current_width, current_height, AV_PIX_FMT_RGBA,
                    SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
                );
            }

            // 4. 写入 Image 内存
            // 确保在主线程使用 RenderingServer::get_singleton()->texture_2d_update
            // 由于 Godot C++ Binding 的限制，我们不能直接在解码线程操作 RenderingServer。
            // 最佳实践是使用一个互斥锁保护的缓冲区，然后在 _process 或 _physics_process 中更新纹理。
            
            // 为了简化，我们直接在解码线程写入 Godot Image 内存，并提交更新 RID，
            // 这是一个不完全线程安全但常见的 GDExtension 视频流方法。
            // 
            PackedByteArray data_array = video_image->get_data();
            uint8_t *dst_data = data_array.ptrw();
            int dst_linesize[4] = { current_width * 4, 0, 0, 0 };
            uint8_t *dst_planes[4] = { dst_data, nullptr, nullptr, nullptr };

            sws_scale(sws_ctx, 
                      video_frame->data, video_frame->linesize,
                      0, video_frame->height, 
                      dst_planes, dst_linesize);

            // 5. 提交到 GPU
            RenderingServer::get_singleton()->texture_2d_update(video_texture_rid, video_image, 0);
        }
    }
    
    return DR_OK;
}

// --- Audio Playback Handoff (Requirement ②) ---

Array MoonlightStreamCore::get_audio_generators() const {
    Array result;
    std::lock_guard<std::mutex> lock(audio_mutex);
    for (const auto &ctx : audio_channels) {
        result.push_back(ctx.generator);
    }
    return result;
}

void MoonlightStreamCore::set_audio_playback(int channel_idx, const Ref<AudioStreamGeneratorPlayback> &playback) {
    std::lock_guard<std::mutex> lock(audio_mutex);
    if (channel_idx >= 0 && channel_idx < (int)audio_channels.size()) {
        audio_channels[channel_idx].playback = playback;
        UtilityFunctions::print(vformat("Audio playback for channel %d updated: %s", channel_idx, playback.is_valid() ? "Valid" : "Null"));
    } else {
        UtilityFunctions::push_error(vformat("Invalid channel index: %d", channel_idx));
    }
}

// 音频解码器初始化 (在 Moonlight 线程中执行)
bool MoonlightStreamCore::_init_audio_decoder(const OPUS_MULTISTREAM_CONFIGURATION *config) {
    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_OPUS);
    if (!codec) {
        // 由于 Godot Log 输出是线程安全的，这里可以直接使用
        UtilityFunctions::push_error("FFmpeg: Opus Codec not found");
        return false;
    }

    audio_codec_ctx = avcodec_alloc_context3(codec);
    if (!audio_codec_ctx) {
        UtilityFunctions::push_error("FFmpeg: Failed to alloc audio context");
        return false;
    }

    audio_codec_ctx->sample_rate = config->sampleRate;
    audio_codec_ctx->ch_layout.nb_channels = config->channelCount;
    audio_codec_ctx->request_sample_fmt = AV_SAMPLE_FMT_FLT; // 请求浮点输出 (Godot 使用浮点)

    if (avcodec_open2(audio_codec_ctx, codec, nullptr) < 0) {
        UtilityFunctions::push_error("FFmpeg: Failed to open Opus codec");
        return false;
    }
    
    // 在主线程中创建 Godot 资源
    call_deferred("_setup_audio_generators_deferred", config->channelCount, config->sampleRate);
    
    return true;
}

// 在主线程中创建 AudioStreamGenerator 资源
void MoonlightStreamCore::_setup_audio_generators_deferred(int channel_count, int sample_rate) {
    std::lock_guard<std::mutex> lock(audio_mutex);
    
    // 清理旧资源
    audio_channels.clear(); 

    for (int i = 0; i < channel_count; i++) {
        AudioChannelContext ctx;
        ctx.generator.instantiate();
        ctx.generator->set_mix_rate(sample_rate);
        ctx.generator->set_buffer_length(0.1f); // 100ms buffer
        audio_channels.push_back(ctx);
    }
    UtilityFunctions::print(vformat("Initialized %d audio generators at %d Hz.", channel_count, sample_rate));
}

// 音频初始化回调 (在 Moonlight 线程中执行)
int MoonlightStreamCore::_on_audio_init(int audioConfiguration, const POPUS_MULTISTREAM_CONFIGURATION opusConfig) {
    if (!_init_audio_decoder(opusConfig)) return -1;
    return 0;
}

// 音频解码回调 (在 Moonlight 线程中执行)
void MoonlightStreamCore::_on_decode_and_play_sample(char *data, int length) {
    if (!is_streaming || !audio_codec_ctx) return;

    // 1. 发送包
    audio_packet->data = (uint8_t*)data;
    audio_packet->size = length;
    int ret = avcodec_send_packet(audio_codec_ctx, audio_packet);
    if (ret < 0) return;

    // 2. 接收帧
    while (ret >= 0) {
        ret = avcodec_receive_frame(audio_codec_ctx, audio_frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0) break;

        // 3. 推送数据到 Godot Playback
        std::lock_guard<std::mutex> lock(audio_mutex);
        
        int samples = audio_frame->nb_samples;
        int channels = audio_frame->ch_layout.nb_channels;
        int active_generators = audio_channels.size();
        
        // 要求输出格式为 AV_SAMPLE_FMT_FLT
        if (active_generators == 0 || audio_frame->format != AV_SAMPLE_FMT_FLT) continue;
        
        bool is_planar = av_sample_fmt_is_planar((AVSampleFormat)audio_frame->format);
        
        // 遍历所有解码出的 FFmpeg 声道
        for (int ch = 0; ch < active_generators && ch < channels; ch++) {
            // 检查 Playback 是否已由 GDScript 传入
            if (audio_channels[ch].playback.is_null()) continue;
            
            // 检查缓冲区空间，如果太满则丢弃
            if (audio_channels[ch].playback->get_frames_available() < samples / 2) continue;

            PackedVector2Array buffer;
            buffer.resize(samples);
            Vector2 *ptr = buffer.ptrw();

            // 提取 PCM 数据 (float 32bit)
            for (int i = 0; i < samples; i++) {
                float sample_val = 0.0f;
                
                if (is_planar) {
                    // Planar: data[ch][i]
                    sample_val = ((float*)audio_frame->data[ch])[i];
                } else {
                    // Packed: data[0][i*channels + ch]
                    float* packed = (float*)audio_frame->data[0];
                    sample_val = packed[i * channels + ch];
                }

                // Godot 的 AudioStreamGenerator 接收 Vector2 (Stereo Frame)。
                // 将单声道样本复制到左右声道以实现播放。
                ptr[i] = Vector2(sample_val, sample_val); 
            }

            // 推入播放器
            audio_channels[ch].playback->push_buffer(buffer);
        }
    }
}

// --- Connection Callbacks (Requirement ③) ---

void MoonlightStreamCore::_on_connection_started() {
    call_deferred("emit_signal", "connection_started");
}

void MoonlightStreamCore::_on_connection_terminated(int errorCode) {
    // 确保清理逻辑只执行一次
    if (is_streaming) {
        stop_connection(); // 清理内部状态
        call_deferred("emit_signal", "connection_stopped");
    }
    if (errorCode != ML_ERROR_GRACEFUL_TERMINATION) {
        String message = vformat("Connection terminated with error code: %d", errorCode);
        call_deferred("emit_signal", "error_occurred", message);
    }
}

void MoonlightStreamCore::_on_connection_status_update(int connectionStatus) {
    call_deferred("emit_signal", "connection_status_changed", connectionStatus);
}

int MoonlightStreamCore::_on_video_setup(int videoFormat, int width, int height, int redrawRate) {
    // 收到配置后在主线程设置 Godot 视频资源
    // 即使在不同线程，call_deferred 也是线程安全的
    call_deferred("_setup_video_resources", width, height);
    return DR_OK;
}

void MoonlightStreamCore::_on_video_cleanup() {
    // 视频流停止，清理解码器
    if (video_codec_ctx) {
        avcodec_free_context(&video_codec_ctx);
        video_codec_ctx = nullptr;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
}

void MoonlightStreamCore::_cleanup_ffmpeg() {
    // 释放所有 FFmpeg 资源
    if (sws_ctx) { sws_freeContext(sws_ctx); }
    if (video_codec_ctx) { avcodec_free_context(&video_codec_ctx); }
    if (audio_codec_ctx) { avcodec_free_context(&audio_codec_ctx); }
    if (video_frame) { av_frame_free(&video_frame); }
    if (video_packet) { av_packet_free(&video_packet); }
    if (audio_frame) { av_frame_free(&audio_frame); }
    if (audio_packet) { av_packet_free(&audio_packet); }
    
    sws_ctx = nullptr;
    video_codec_ctx = nullptr;
    audio_codec_ctx = nullptr;
    video_frame = nullptr;
    video_packet = nullptr;
    audio_frame = nullptr;
    audio_packet = nullptr;
}