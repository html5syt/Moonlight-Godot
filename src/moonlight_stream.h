#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/sub_viewport.hpp"
#include "godot_cpp/classes/audio_stream_generator.hpp"
#include "godot_cpp/classes/audio_stream_generator_playback.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/rendering_server.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/variant/vector2i.hpp"

extern "C" {
#include "lib/moonlight-common-c/src/Limelight.h"
}

#include <map>
#include <mutex>
#include <cstring>
#include <cstdint>
#include <thread>
#include <atomic>

using namespace godot;

class MoonlightStream : public Node {
    GDCLASS(MoonlightStream, Node)

public:
    // 对应 Limelight 的 VIDEO_FORMAT_* 掩码
    enum Codec {
        CODEC_H264 = VIDEO_FORMAT_H264,
        CODEC_HEVC = VIDEO_FORMAT_H265,
        CODEC_AV1  = VIDEO_FORMAT_AV1_MAIN8,
    };

    // 对应 Limelight 的 COLORSPACE_*
    enum ColorSpace {
        COLOR_SPACE_REC_601  = COLORSPACE_REC_601,
        COLOR_SPACE_REC_709  = COLORSPACE_REC_709,
        COLOR_SPACE_REC_2020 = COLORSPACE_REC_2020,
    };

private:
    static void _bind_methods();

    // --- 线程与状态控制 ---
    std::atomic<bool> streaming = false;   // 流是否正在运行
    std::thread       connection_thread;   // 后台连接线程

    // 后台线程函数：执行阻塞的 LiStartConnection()
    void _connection_thread_func();

    // --- 配置参数 ---
    String     host_address;
    int        app_id        = 0;
    int        width         = 1920;
    int        height        = 1080;
    int        fps           = 60;
    int        bitrate_kbps  = 20000;
    Codec      video_codec   = CODEC_H264;
    ColorSpace color_space   = COLOR_SPACE_REC_709;
    bool       enable_hdr    = false;

    // === 来自 /serverinfo ===
    String server_app_version;
    int    server_codec_mode_support = 0;

    // === 来自 /launch (sessionUrl0) ===
    String server_rtsp_session_url;

    // --- 渲染资源 ---
    SubViewport *viewport = nullptr;
    RID          texture_rid;

    // --- 帧队列 (解码线程 -> 主线程) ---
    PackedByteArray pending_frame;
    bool            has_pending_frame = false;
    mutable std::mutex frame_mutex;

    // --- 远程输入加密密钥 (必须由 GDScript 设置) ---
    godot::PackedByteArray custom_aes_key; // 16 bytes
    godot::PackedByteArray custom_aes_iv;  // >= 4 bytes

public:
    MoonlightStream();
    ~MoonlightStream() override;

    // C 回调通过 context 找实例
    static std::map<void*, MoonlightStream*> instance_map;

    // Audio
    Ref<AudioStreamGenerator>         audio_stream;
    Ref<AudioStreamGeneratorPlayback> audio_playback;

    // Key Setters
    void set_remote_input_aes_key(const godot::PackedByteArray &p_key);
    void set_remote_input_aes_iv(const godot::PackedByteArray &p_iv);

    // --- 属性接口（配合 Godot 导出） ---
    void   set_host_address(const String &p_host) { host_address = p_host; }
    String get_host_address() const               { return host_address; }

    void set_app_id(int p_app_id) { app_id = p_app_id; }
    int  get_app_id() const       { return app_id; }

    void   set_server_app_version(const String &p_version) { server_app_version = p_version; }
    String get_server_app_version() const                  { return server_app_version; }

    void set_server_codec_mode_support(int p_support) { server_codec_mode_support = p_support; }
    int  get_server_codec_mode_support() const        { return server_codec_mode_support; }

    void   set_server_rtsp_session_url(const String &p_url) { server_rtsp_session_url = p_url; }
    String get_server_rtsp_session_url() const              { return server_rtsp_session_url; }

    void     set_resolution(int p_width, int p_height);
    Vector2i get_resolution() const { return Vector2i(width, height); }

    void set_fps(int p_fps) { fps = p_fps; }
    int  get_fps() const    { return fps; }

    void set_bitrate_kbps(int p_bitrate) { bitrate_kbps = p_bitrate; }
    int  get_bitrate_kbps() const        { return bitrate_kbps; }

    void  set_video_codec(Codec p_codec)      { video_codec = p_codec; }
    Codec get_video_codec() const             { return video_codec; }

    void       set_color_space(ColorSpace p_color_space) { color_space = p_color_space; }
    ColorSpace get_color_space() const                   { return color_space; }

    void set_enable_hdr(bool p_enable) { enable_hdr = p_enable; }
    bool get_enable_hdr() const        { return enable_hdr; }

    bool                        is_streaming() const         { return streaming.load(); }
    Ref<AudioStreamGenerator>   get_audio_stream() const     { return audio_stream; }

    // --- 控制接口 ---
    void start_connection();   // 通过信号反馈成功或失败
    void stop_connection();

    // --- 输入事件 ---
    void send_mouse_button_event(int button, bool pressed);
    void send_mouse_move_event(float x, float y);
    void send_key_event(int scancode, bool pressed);

    // --- 内部：供 C 回调调用 ---
    int  _on_submit_decode_unit(PDECODE_UNIT decode_unit);
    void _process(double delta) override;

    // --- 内部：从静态回调中转到 Godot 信号 ---
    void _emit_connection_started();
    void _emit_connection_terminated(int error_code);
};

VARIANT_ENUM_CAST(MoonlightStream::Codec);
VARIANT_ENUM_CAST(MoonlightStream::ColorSpace);
