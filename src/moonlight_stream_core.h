#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/audio_stream_generator.hpp>
#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_vector2_array.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <vector>
#include <mutex>
#include <atomic>
#include <map>
#include <cstring>
#include <thread>

// FFmpeg headers
// 假设这些头文件已在项目中配置好
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/samplefmt.h> // for AV_SAMPLE_FMT_FLT
#include <libavutil/channel_layout.h>
}

// 实际链接到用户提供的 Limelight.h
// 为了代码简洁和可读性，这里假设 Limelight.h 是在 include 路径下的。
// 如果用户上传的文件名为 Limelight - 副本.h，则应调整为 #include "Limelight - 副本.h"
extern "C" {
#include "lib/moonlight-common-c/src/Limelight.h"
}
using namespace godot;

class MoonlightStreamCore : public Node {
    GDCLASS(MoonlightStreamCore, Node)

public:
    // 对应 Limelight 的 STREAM_CFG_*
    enum RemoteMode {
        REMOTE_LOCAL = STREAM_CFG_LOCAL,
        REMOTE_REMOTE = STREAM_CFG_REMOTE,
        REMOTE_AUTO = STREAM_CFG_AUTO
    };

private:
    // --- Godot Video Resources ---
    SubViewport *sub_viewport = nullptr;
    TextureRect *video_display_rect = nullptr;
    Ref<ImageTexture> video_texture;
    Ref<Image> video_image;
    RID video_texture_rid; 
    
    // --- Audio Resources (Requirement ②) ---
    struct AudioChannelContext {
        // AudioStreamGenerator 用于 GDScript 绑定
        Ref<AudioStreamGenerator> generator;
        // AudioStreamGeneratorPlayback 用于 C++ 推送数据
        Ref<AudioStreamGeneratorPlayback> playback; 
    };
    std::vector<AudioChannelContext> audio_channels;
    mutable std::mutex audio_mutex; // 保护音频通道列表和 playback 的线程安全

    // --- FFmpeg Contexts ---
    AVCodecContext *video_codec_ctx = nullptr;
    AVFrame        *video_frame = nullptr;
    AVPacket       *video_packet = nullptr;
    SwsContext     *sws_ctx = nullptr; // For YUV to RGBA conversion

    AVCodecContext *audio_codec_ctx = nullptr;
    AVFrame        *audio_frame = nullptr;
    AVPacket       *audio_packet = nullptr;
    

    
    int current_width = 0;
    int current_height = 0;
    
    // --- Internal Logic ---
    void _cleanup_ffmpeg();
    void _setup_video_resources(int width, int height);
    bool _init_video_decoder(PDECODE_UNIT du);
    bool _init_audio_decoder(const OPUS_MULTISTREAM_CONFIGURATION *config);
    void _setup_audio_generators_deferred(int channel_count, int sample_rate); // 在主线程中调用

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    MoonlightStreamCore();
    ~MoonlightStreamCore();
    // --- Internal State & Moonlight Context ---
    // 用于将 C 回调的 void* context 映射回 C++ 实例
    static std::map<void *, MoonlightStreamCore *> instance_map;
    // --- Connection Interface (Requirement ③) ---
    void start_connection(const String &address, const Dictionary &config);
    void stop_connection();
    std::atomic<bool> is_streaming = false;

    // --- Accessors & Audio Playback Handoff ---
    SubViewport *get_video_viewport() const;
    Array get_audio_generators() const; // 返回 Array[AudioStreamGenerator]
    
    // 关键 API：GDScript 调用此方法将激活的 Playback 对象传回 C++ Core (Requirement ②)
    void set_audio_playback(int channel_idx, const Ref<AudioStreamGeneratorPlayback> &playback);

    // --- Internal Callbacks (C-style wrappers will call these) ---
    // Video Callbacks
    int  _on_video_setup(int videoFormat, int width, int height, int redrawRate);
    void _on_video_cleanup();
    int  _on_submit_decode_unit(PDECODE_UNIT decodeUnit);

    // Audio Callbacks
    int  _on_audio_init(int audioConfiguration, const POPUS_MULTISTREAM_CONFIGURATION opusConfig);
    void _on_decode_and_play_sample(char *sampleData, int sampleLength);

    // Connection Callbacks
    void _on_connection_started();
    void _on_connection_terminated(int errorCode);
    void _on_connection_status_update(int connectionStatus);
};