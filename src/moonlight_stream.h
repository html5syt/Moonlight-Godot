#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/sub_viewport.hpp"
#include "godot_cpp/classes/audio_stream_generator.hpp"
#include "godot_cpp/classes/audio_stream_generator_playback.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"

extern "C" {
#include "lib/moonlight-common-c/src/Limelight.h"
}

#include <map>
#include <mutex>

using namespace godot;

class MoonlightStream : public Node {
    GDCLASS(MoonlightStream, Node)

public:
    enum Codec {
        CODEC_H264 = VIDEO_FORMAT_H264,
        CODEC_HEVC = VIDEO_FORMAT_H265,
        CODEC_AV1 = VIDEO_FORMAT_AV1_MAIN8,
    };

    enum ColorSpace {
        COLOR_SPACE_REC_601 = COLORSPACE_REC_601,
        COLOR_SPACE_REC_709 = COLORSPACE_REC_709,
        COLOR_SPACE_REC_2020 = COLORSPACE_REC_2020,
    };

private:
    static void _bind_methods();

    bool streaming = false;
    String host_address;
    int app_id = 0;
    int width = 1920;
    int height = 1080;
    int fps = 60;
    int bitrate_kbps = 20000;
    Codec video_codec = CODEC_H264;
    ColorSpace color_space = COLOR_SPACE_REC_709;
    bool enable_hdr = false;

    // === 新增：服务器信息（由用户从 /serverinfo 提供）===
    String server_app_version;           // 来自 /serverinfo 的 appversion
    int server_codec_mode_support = 0;   // 来自 /serverinfo 的 ServerCodecModeSupport

    SubViewport* viewport = nullptr;
    RID texture_rid;


    // Frame queue (decoder thread → main thread)
    PackedByteArray pending_frame;
    bool has_pending_frame = false;
    mutable std::mutex frame_mutex;



public:
    MoonlightStream();
    ~MoonlightStream() override;
    // Static map for context → instance
    static std::map<void*, MoonlightStream*> instance_map;
    // Audio
    Ref<AudioStreamGenerator> audio_stream;
    Ref<AudioStreamGeneratorPlayback> audio_playback;
    // Properties
    void set_host_address(const String& p_host) { host_address = p_host; }
    String get_host_address() const { return host_address; }

    void set_app_id(int p_app_id) { app_id = p_app_id; }
    int get_app_id() const { return app_id; }

    void set_server_app_version(const String& p_version) { server_app_version = p_version; }
    String get_server_app_version() const { return server_app_version; }

    void set_server_codec_mode_support(int p_support) { server_codec_mode_support = p_support; }
    int get_server_codec_mode_support() const { return server_codec_mode_support; }

    void set_resolution(int p_width, int p_height);
    void get_resolution(int& r_width, int& r_height) const { r_width = width; r_height = height; }

    void set_fps(int p_fps) { fps = p_fps; }
    int get_fps() const { return fps; }

    void set_bitrate_kbps(int p_bitrate) { bitrate_kbps = p_bitrate; }
    int get_bitrate_kbps() const { return bitrate_kbps; }

    void set_video_codec(Codec p_codec) { video_codec = p_codec; }
    Codec get_video_codec() const { return video_codec; }

    void set_color_space(ColorSpace p_color_space) { color_space = p_color_space; }
    ColorSpace get_color_space() const { return color_space; }

    void set_enable_hdr(bool p_enable) { enable_hdr = p_enable; }
    bool get_enable_hdr() const { return enable_hdr; }

    bool is_streaming() const { return streaming; }

    Ref<AudioStreamGenerator> get_audio_stream() const { return audio_stream; }

    // Control
    bool start_connection();
    void stop_connection();

    // Input
    void send_mouse_button_event(int button, bool pressed);
    void send_mouse_move_event(float x, float y);
    void send_key_event(int scancode, bool pressed);

    // Internal
    int _on_submit_decode_unit(PDECODE_UNIT decode_unit);
    void _process(double delta) override;
};

VARIANT_ENUM_CAST(MoonlightStream::Codec);
VARIANT_ENUM_CAST(MoonlightStream::ColorSpace);