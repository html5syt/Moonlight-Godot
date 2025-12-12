#include "moonlight_stream.h"

using namespace godot;

// ========= 静态全局 =========

// context -> instance 映射（给视频/音频回调用）
std::map<void *, MoonlightStream *> MoonlightStream::instance_map;

// 当前激活连接实例（给 ConnectionListener 回调用，因其没有 context）
static MoonlightStream *global_active_stream = nullptr;

// 每个解码线程自己的当前实例
thread_local MoonlightStream *tls_current_stream = nullptr;


// ========= 静态视频回调 (DecoderRendererCallbacks) =========

// setup(videoFormat, width, height, redrawRate, context, drFlags)
static int _dr_setup(int video_format, int width, int height,
                     int redraw_rate, void *context, int dr_flags) {
    (void)video_format;
    (void)redraw_rate;
    (void)dr_flags;

    auto it = MoonlightStream::instance_map.find(context);
    if (it != MoonlightStream::instance_map.end()) {
        tls_current_stream = it->second;
        // 在主线程上更新分辨率
        tls_current_stream->call_deferred("set_resolution", width, height);
    } else {
        tls_current_stream = nullptr;
    }
    return DR_OK;
}

static void _dr_cleanup(void) {
    tls_current_stream = nullptr;
}

// submitDecodeUnit(PDECODE_UNIT du)
static int _dr_submit_decode_unit(PDECODE_UNIT decode_unit) {
    if (tls_current_stream && tls_current_stream->is_streaming()) {
        return tls_current_stream->_on_submit_decode_unit(decode_unit);
    }
    return DR_OK;
}

// ========= 静态音频回调 (AudioRendererCallbacks) =========

// init(audioConfiguration, opusConfig, context, arFlags)
static int _ar_init(int audio_configuration,
                    const POPUS_MULTISTREAM_CONFIGURATION opus_config,
                    void *context, int ar_flags) {
    (void)audio_configuration;
    (void)opus_config;
    (void)ar_flags;

    auto it = MoonlightStream::instance_map.find(context);
    if (it == MoonlightStream::instance_map.end()) {
        return -1;
    }

    MoonlightStream *self = it->second;

    if (self->audio_stream.is_valid()) {
        self->audio_playback = self->audio_stream->instantiate_playback();
    }

    return 0;
}

// decodeAndPlaySample(sampleData, sampleLength)
static void _ar_decode_and_play_sample(char *sample_data, int sample_length) {
    // 因为 AudioRenderer 回调没有 context，这里简单找第一个有 playback 的实例
    for (auto &pair : MoonlightStream::instance_map) {
        MoonlightStream *self = pair.second;
        if (!self || !self->audio_playback.is_valid()) {
            continue;
        }

        const int16_t *samples      = reinterpret_cast<int16_t *>(sample_data);
        const int      sample_count = sample_length / static_cast<int>(sizeof(int16_t));
        const int      frame_count  = sample_count / 2; // 双声道

        PackedVector2Array buffer;
        buffer.resize(frame_count);
        Vector2 *write_ptr = buffer.ptrw();

        for (int i = 0; i < frame_count; ++i) {
            const float left  = static_cast<float>(samples[i * 2 + 0]) / 32768.0f;
            const float right = static_cast<float>(samples[i * 2 + 1]) / 32768.0f;
            write_ptr[i] = Vector2(left, right);
        }

        self->audio_playback->push_buffer(buffer);
        return;
    }
}

// ========= Godot 绑定 =========
void MoonlightStream::_bind_methods() {
    // 属性绑定
    ClassDB::bind_method(D_METHOD("set_host_address", "host"),
                         &MoonlightStream::set_host_address);
    ClassDB::bind_method(D_METHOD("get_host_address"),
                         &MoonlightStream::get_host_address);

    ClassDB::bind_method(D_METHOD("set_app_id", "app_id"),
                         &MoonlightStream::set_app_id);
    ClassDB::bind_method(D_METHOD("get_app_id"),
                         &MoonlightStream::get_app_id);

    ClassDB::bind_method(D_METHOD("set_server_app_version", "version"),
                         &MoonlightStream::set_server_app_version);
    ClassDB::bind_method(D_METHOD("get_server_app_version"),
                         &MoonlightStream::get_server_app_version);

    ClassDB::bind_method(D_METHOD("set_server_codec_mode_support", "support"),
                         &MoonlightStream::set_server_codec_mode_support);
    ClassDB::bind_method(D_METHOD("get_server_codec_mode_support"),
                         &MoonlightStream::get_server_codec_mode_support);

    ClassDB::bind_method(D_METHOD("set_server_rtsp_session_url", "url"),
                         &MoonlightStream::set_server_rtsp_session_url);
    ClassDB::bind_method(D_METHOD("get_server_rtsp_session_url"),
                         &MoonlightStream::get_server_rtsp_session_url);

    ClassDB::bind_method(D_METHOD("set_resolution", "width", "height"),
                         &MoonlightStream::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"),
                         &MoonlightStream::get_resolution);

    ClassDB::bind_method(D_METHOD("set_fps", "fps"),
                         &MoonlightStream::set_fps);
    ClassDB::bind_method(D_METHOD("get_fps"),
                         &MoonlightStream::get_fps);

    ClassDB::bind_method(D_METHOD("set_bitrate_kbps", "bitrate_kbps"),
                         &MoonlightStream::set_bitrate_kbps);
    ClassDB::bind_method(D_METHOD("get_bitrate_kbps"),
                         &MoonlightStream::get_bitrate_kbps);

    ClassDB::bind_method(D_METHOD("set_video_codec", "codec"),
                         &MoonlightStream::set_video_codec);
    ClassDB::bind_method(D_METHOD("get_video_codec"),
                         &MoonlightStream::get_video_codec);

    ClassDB::bind_method(D_METHOD("set_color_space", "color_space"),
                         &MoonlightStream::set_color_space);
    ClassDB::bind_method(D_METHOD("get_color_space"),
                         &MoonlightStream::get_color_space);

    ClassDB::bind_method(D_METHOD("set_enable_hdr", "enable"),
                         &MoonlightStream::set_enable_hdr);
    ClassDB::bind_method(D_METHOD("get_enable_hdr"),
                         &MoonlightStream::get_enable_hdr);

    ClassDB::bind_method(D_METHOD("get_audio_stream"),
                         &MoonlightStream::get_audio_stream);

    // 控制
    ClassDB::bind_method(D_METHOD("start_connection"),
                         &MoonlightStream::start_connection);
    ClassDB::bind_method(D_METHOD("stop_connection"),
                         &MoonlightStream::stop_connection);

    // 输入
    ClassDB::bind_method(D_METHOD("send_mouse_button_event", "button", "pressed"),
                         &MoonlightStream::send_mouse_button_event);
    ClassDB::bind_method(D_METHOD("send_mouse_move_event", "x", "y"),
                         &MoonlightStream::send_mouse_move_event);
    ClassDB::bind_method(D_METHOD("send_key_event", "scancode", "pressed"),
                         &MoonlightStream::send_key_event);

    // AES key/iv
    ClassDB::bind_method(D_METHOD("set_remote_input_aes_key", "key"),
                         &MoonlightStream::set_remote_input_aes_key);
    ClassDB::bind_method(D_METHOD("set_remote_input_aes_iv", "iv"),
                         &MoonlightStream::set_remote_input_aes_iv);

    // 属性导出到 Inspector
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_address"),
                 "set_host_address", "get_host_address");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "app_id"),
                 "set_app_id", "get_app_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_app_version"),
                 "set_server_app_version", "get_server_app_version");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "server_codec_mode_support"),
                 "set_server_codec_mode_support", "get_server_codec_mode_support");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_rtsp_session_url"),
                 "set_server_rtsp_session_url", "get_server_rtsp_session_url");

    ADD_PROPERTY(PropertyInfo(Variant::INT, "fps",
                              PROPERTY_HINT_RANGE, "10,120,1"),
                 "set_fps", "get_fps");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "bitrate_kbps",
                              PROPERTY_HINT_RANGE, "500,100000,1"),
                 "set_bitrate_kbps", "get_bitrate_kbps");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "video_codec",
                              PROPERTY_HINT_ENUM, "H264,HEVC,AV1"),
                 "set_video_codec", "get_video_codec");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "color_space",
                              PROPERTY_HINT_ENUM, "Rec601,Rec709,Rec2020"),
                 "set_color_space", "get_color_space");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_hdr"),
                 "set_enable_hdr", "get_enable_hdr");

    // 枚举导出
    BIND_ENUM_CONSTANT(CODEC_H264);
    BIND_ENUM_CONSTANT(CODEC_HEVC);
    BIND_ENUM_CONSTANT(CODEC_AV1);

    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_601);
    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_709);
    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_2020);

    // 信号
    ADD_SIGNAL(MethodInfo("connection_started"));
    ADD_SIGNAL(MethodInfo("connection_terminated",
                          PropertyInfo(Variant::INT, "error_code")));
    ADD_SIGNAL(MethodInfo("connection_failed",
                          PropertyInfo(Variant::STRING, "message")));
    ADD_SIGNAL(MethodInfo("video_packet_received", 
        PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data"),
        PropertyInfo(Variant::INT, "frame_type")));
}

// ========= 构造 / 析构 =========
MoonlightStream::MoonlightStream() {
    void *ctx = this;
    instance_map[ctx] = this;

    // Audio generator
    audio_stream.instantiate();
    audio_stream->set_mix_rate(48000); // Moonlight 默认 48k

}

MoonlightStream::~MoonlightStream() {
    instance_map.erase(this);
    stop_connection();
    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }
}

// ========= 属性 =========
void MoonlightStream::set_resolution(int p_width, int p_height) {
    width  = p_width;
    height = p_height;
}

// ========= 连接控制 =========
void MoonlightStream::start_connection() {
    if (streaming.load()) {
        UtilityFunctions::push_warning("MoonlightStream: Already streaming.");
        return;
    }

    // 上一次线程若仍存在，先 join
    if (connection_thread.joinable()) {
        connection_thread.join();
    }

    // 配置合法性检查
    if (host_address.is_empty()) {
        emit_signal("connection_failed", String("Host address is empty"));
        return;
    }

    if (custom_aes_key.size() != 16) {
        emit_signal("connection_failed", String("Invalid AES key (must be 16 bytes)"));
        return;
    }

    if (custom_aes_iv.size() < 4) {
        emit_signal("connection_failed", String("Invalid AES IV (must be >= 4 bytes)"));
        return;
    }

    if (server_app_version.is_empty()) {
        emit_signal("connection_failed", String("Server app version is not set"));
        return;
    }

    global_active_stream = this;
    streaming.store(true);

    // 启动后台线程
    connection_thread = std::thread(&MoonlightStream::_connection_thread_func, this);
}

void MoonlightStream::_connection_thread_func() {
    // --- SERVER_INFORMATION ---
    SERVER_INFORMATION server_info;
    LiInitializeServerInformation(&server_info);

    CharString host_str    = host_address.utf8();
    CharString app_ver_str = server_app_version.utf8();
    CharString rtsp_str    = server_rtsp_session_url.utf8();

    server_info.address            = host_str.get_data();
    server_info.serverInfoAppVersion = app_ver_str.get_data();
    server_info.serverInfoGfeVersion  = nullptr; // 可选字段
    server_info.rtspSessionUrl       = server_rtsp_session_url.is_empty()
                                        ? nullptr
                                        : rtsp_str.get_data();
    server_info.serverCodecModeSupport = server_codec_mode_support;

    // --- STREAM_CONFIGURATION ---
    STREAM_CONFIGURATION stream_cfg;
    LiInitializeStreamConfiguration(&stream_cfg);

    stream_cfg.width               = width;
    stream_cfg.height              = height;
    stream_cfg.fps                 = fps;
    stream_cfg.bitrate             = bitrate_kbps;
    stream_cfg.packetSize          = 1392; // 常见值，避免 MTU 问题
    stream_cfg.streamingRemotely   = STREAM_CFG_LOCAL;
    stream_cfg.audioConfiguration  = AUDIO_CONFIGURATION_STEREO;
    stream_cfg.supportedVideoFormats = static_cast<int>(video_codec);
    stream_cfg.clientRefreshRateX100 = 0; // 不指定
    stream_cfg.colorSpace          = static_cast<int>(color_space);
    stream_cfg.colorRange          = enable_hdr ? COLOR_RANGE_FULL : COLOR_RANGE_LIMITED;
    stream_cfg.encryptionFlags     = ENCFLG_ALL;

    // 远程输入 AES 密钥
    std::memset(stream_cfg.remoteInputAesKey, 0, sizeof(stream_cfg.remoteInputAesKey));
    std::memset(stream_cfg.remoteInputAesIv,  0, sizeof(stream_cfg.remoteInputAesIv));

    std::memcpy(stream_cfg.remoteInputAesKey, custom_aes_key.ptr(), 16);
    std::memcpy(stream_cfg.remoteInputAesIv,  custom_aes_iv.ptr(), 4);
    // 后 12 字节保持 0

    // --- 视频回调 ---
    DECODER_RENDERER_CALLBACKS dr_cb;
    LiInitializeVideoCallbacks(&dr_cb);
    dr_cb.setup            = _dr_setup;
    dr_cb.cleanup          = _dr_cleanup;
    dr_cb.submitDecodeUnit = _dr_submit_decode_unit;
    dr_cb.capabilities     = CAPABILITY_DIRECT_SUBMIT;

    // --- 音频回调 ---
    AUDIO_RENDERER_CALLBACKS ar_cb;
    LiInitializeAudioCallbacks(&ar_cb);
    ar_cb.init                 = _ar_init;
    ar_cb.decodeAndPlaySample  = _ar_decode_and_play_sample;
    ar_cb.capabilities         = CAPABILITY_DIRECT_SUBMIT;

    // --- 连接监听 ---
    CONNECTION_LISTENER_CALLBACKS cl_cb;
    LiInitializeConnectionCallbacks(&cl_cb);

    cl_cb.stageStarting = [](int stage) {
        UtilityFunctions::print("Moonlight stage starting: ", stage);
    };
    cl_cb.stageFailed = [](int stage, int error) {
        UtilityFunctions::push_warning(
            vformat("Moonlight stage %d failed: %d", stage, error));
    };
    cl_cb.connectionStarted = []() {
        UtilityFunctions::print("Moonlight connection started.");
        if (global_active_stream) {
            global_active_stream->_emit_connection_started();
        }
    };
    cl_cb.connectionTerminated = [](int error) {
        UtilityFunctions::print("Moonlight connection terminated: ", error);
        if (global_active_stream) {
            // 在线程结束时，通过 call_deferred 统一发信号
            global_active_stream->call_deferred("_emit_connection_terminated", error);
        }
    };
    // 其他回调保持 NULL

    void *render_ctx = this;
    void *audio_ctx  = this;

    // 阻塞直到连接结束
    int ret = LiStartConnection(
        &server_info,
        &stream_cfg,
        &cl_cb,
        &dr_cb,
        &ar_cb,
        render_ctx,
        0,           // drFlags
        audio_ctx,
        0            // arFlags
    );

    streaming.store(false);
    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }

    // 若 connectionTerminated 没被调用（比如早期阶段失败），在这里补一次
    call_deferred("_emit_connection_terminated", ret);
}

void MoonlightStream::stop_connection() {
    // 1. 发送停止请求
    if (streaming.load()) {
        LiStopConnection();
    }

    // 2. 等待线程结束
    if (connection_thread.joinable()) {
        connection_thread.join();
    }

    streaming.store(false);

    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }
}

// ========= 信号中转 =========
void MoonlightStream::_emit_connection_started() {
    // 直接在主线程 emit，如果是从回调来的，用 call_deferred 调用这个函数
    emit_signal("connection_started");
}

void MoonlightStream::_emit_connection_terminated(int error_code) {
    emit_signal("connection_terminated", error_code);
}

// ========= 解码帧提交 (视频回调线程) =========
int MoonlightStream::_on_submit_decode_unit(PDECODE_UNIT decode_unit) {
    if (!streaming.load()) {
        return DR_OK;
    }

    // 1. 计算总数据长度
    int total_len = decode_unit->fullLength;
    if (total_len <= 0) return DR_OK;

    // 2. 准备 PackedByteArray
    PackedByteArray packet_data;
    packet_data.resize(total_len);
    uint8_t *ptr = packet_data.ptrw();

    // 3. 拼接 buffer 链表
    // Limelight 将一帧数据拆分成了多个链表节点，我们需要把它们拼成一个完整的 H.264/HEVC 帧
    PLENTRY entry = decode_unit->bufferList;
    int offset = 0;
    while (entry != nullptr) {
        if (entry->length > 0 && entry->data != nullptr) {
            // 安全检查防止溢出
            if (offset + entry->length <= total_len) {
                std::memcpy(ptr + offset, entry->data, entry->length);
                offset += entry->length;
            }
        }
        entry = entry->next;
    }

    // 4. 发送信号到 C#
    // 注意：这个函数是在解码线程调用的。
    // Godot 4.x 的 emit_signal 在多线程环境下通常是线程安全的，它会将信号放入主线程队列。
    // 如果遇到问题，可以使用 call_deferred("_emit_video_packet", ...)
    
    // 这里为了性能，尽量直接 emit，让 C# 端决定是否在线程池处理
    call_deferred("_emit_video_packet", packet_data, decode_unit->frameType);

    return DR_OK;
}

void MoonlightStream::_emit_video_packet(const PackedByteArray &data, int frame_type) {
    emit_signal("video_packet_received", data, frame_type);
}



// ========= 输入事件 =========
void MoonlightStream::send_mouse_button_event(int button, bool pressed) {
    if (!streaming.load()) {
        return;
    }

    LiSendMouseButtonEvent(
        pressed ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE,
        button);
}

void MoonlightStream::send_mouse_move_event(float x, float y) {
    if (!streaming.load()) {
        return;
    }

    LiSendMouseMoveEvent(static_cast<short>(x), static_cast<short>(y));
}

void MoonlightStream::send_key_event(int scancode, bool pressed) {
    if (!streaming.load()) {
        return;
    }

    LiSendKeyboardEvent(
        static_cast<short>(scancode),
        pressed ? KEY_ACTION_DOWN : KEY_ACTION_UP,
        0);
}

// ========= 远程输入 AES =========
void MoonlightStream::set_remote_input_aes_key(const PackedByteArray &p_key) {
    custom_aes_key = p_key;
}

void MoonlightStream::set_remote_input_aes_iv(const PackedByteArray &p_iv) {
    custom_aes_iv = p_iv;
}
