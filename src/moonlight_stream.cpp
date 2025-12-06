#include "moonlight_stream.h"

// Static map for context -> instance (用于音视频回调)
std::map<void*, MoonlightStream*> MoonlightStream::instance_map;

// Global pointer for connection listeners (因为 C 回调不带 context)
// 注意：这意味着同一时间只能有一个活跃的连接实例，这符合 Moonlight 的典型用法
static MoonlightStream* global_active_stream = nullptr;

// Thread-local storage for current stream instance (per decoder thread)
thread_local MoonlightStream* tls_current_stream = nullptr;

// ============ YUV420P to RGBA (BT.601) ============
inline uint8_t clamp(int x) { return static_cast<uint8_t>(x < 0 ? 0 : (x > 255 ? 255 : x)); }

void yuv420p_to_rgba(const uint8_t* y, const uint8_t* u, const uint8_t* v, uint8_t* rgba, int width, int height) {
    int uv_width = width / 2;
    int uv_height = height / 2;
    for (int yy = 0; yy < height; ++yy) {
        for (int xx = 0; xx < width; ++xx) {
            int uv_xx = xx >> 1;
            int uv_yy = yy >> 1;
            int Y = y[yy * width + xx];
            int U = u[uv_yy * uv_width + uv_xx] - 128;
            int V = v[uv_yy * uv_width + uv_xx] - 128;
            int R = Y + ((359 * V) >> 8);
            int G = Y - ((88 * U + 183 * V) >> 8);
            int B = Y + ((454 * U) >> 8);
            int idx = (yy * width + xx) * 4;
            rgba[idx + 0] = clamp(R);
            rgba[idx + 1] = clamp(G);
            rgba[idx + 2] = clamp(B);
            rgba[idx + 3] = 255;
        }
    }
}

// ============ Video Callbacks (STATIC) ============
static int dr_setup(int video_format, int width, int height, int redraw_rate, void* context, int dr_flags) {
    auto it = MoonlightStream::instance_map.find(context);
    if (it != MoonlightStream::instance_map.end()) {
        tls_current_stream = it->second;
        // 使用 call_deferred 确保在主线程更新 UI 属性
        tls_current_stream->call_deferred("set_resolution", width, height);
    } else {
        tls_current_stream = nullptr;
    }
    return DR_OK;
}

static void dr_cleanup(void) {
    tls_current_stream = nullptr;
}

static int dr_submit_decode_unit(PDECODE_UNIT decode_unit) {
    if (tls_current_stream && tls_current_stream->is_streaming()) {
        return tls_current_stream->_on_submit_decode_unit(decode_unit);
    }
    return DR_OK;
}

// ============ Audio Callbacks (STATIC) ============
static int ar_init(int audio_configuration, const POPUS_MULTISTREAM_CONFIGURATION opus_config, void* context, int ar_flags) {
    auto it = MoonlightStream::instance_map.find(context);
    if (it == MoonlightStream::instance_map.end()) {
        return -1;
    }
    MoonlightStream* self = it->second;
    
    // 假设 audio_stream 已经在主线程初始化完毕 (在构造函数中)
    if (self->audio_stream.is_valid()) {
        // instantiate_playback 是线程安全的吗？通常建议在主线程做
        // 这里为了简化直接调用，如果崩溃需移至 _connection_thread_func 前的主线程逻辑
        self->audio_playback = self->audio_stream->instantiate_playback();
    }
    return 0;
}

static void ar_decode_and_play_sample(char* sample_data, int sample_length) {
    for (auto& pair : MoonlightStream::instance_map) {
        MoonlightStream* self = pair.second;
        if (self && self->audio_playback.is_valid()) {
            int sample_count = sample_length / sizeof(int16_t);
            int frame_count = sample_count / 2;
            PackedVector2Array buffer;
            buffer.resize(frame_count);
            Vector2* write_ptr = buffer.ptrw();
            int16_t* samples = reinterpret_cast<int16_t*>(sample_data);
            for (int i = 0; i < frame_count; ++i) {
                float left = static_cast<float>(samples[i * 2]) / 32768.0f;
                float right = static_cast<float>(samples[i * 2 + 1]) / 32768.0f;
                write_ptr[i] = Vector2(left, right);
            }
            self->audio_playback->push_buffer(buffer);
            return;
        }
    }
}

// ============ Godot Binding ============
void MoonlightStream::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_host_address", "host"), &MoonlightStream::set_host_address);
    ClassDB::bind_method(D_METHOD("get_host_address"), &MoonlightStream::get_host_address);
    ClassDB::bind_method(D_METHOD("set_app_id", "app_id"), &MoonlightStream::set_app_id);
    ClassDB::bind_method(D_METHOD("get_app_id"), &MoonlightStream::get_app_id);
    ClassDB::bind_method(D_METHOD("set_server_app_version", "version"), &MoonlightStream::set_server_app_version);
    ClassDB::bind_method(D_METHOD("get_server_app_version"), &MoonlightStream::get_server_app_version);
    ClassDB::bind_method(D_METHOD("set_server_codec_mode_support", "support"), &MoonlightStream::set_server_codec_mode_support);
    ClassDB::bind_method(D_METHOD("get_server_codec_mode_support"), &MoonlightStream::get_server_codec_mode_support);
    ClassDB::bind_method(D_METHOD("set_server_rtsp_session_url", "url"), &MoonlightStream::set_server_rtsp_session_url);
    ClassDB::bind_method(D_METHOD("get_server_rtsp_session_url"), &MoonlightStream::get_server_rtsp_session_url);
    ClassDB::bind_method(D_METHOD("set_resolution", "width", "height"), &MoonlightStream::set_resolution);
    ClassDB::bind_method(D_METHOD("get_resolution"), &MoonlightStream::get_resolution);
    ClassDB::bind_method(D_METHOD("set_fps", "fps"), &MoonlightStream::set_fps);
    ClassDB::bind_method(D_METHOD("get_fps"), &MoonlightStream::get_fps);
    ClassDB::bind_method(D_METHOD("set_bitrate_kbps", "bitrate_kbps"), &MoonlightStream::set_bitrate_kbps);
    ClassDB::bind_method(D_METHOD("get_bitrate_kbps"), &MoonlightStream::get_bitrate_kbps);
    ClassDB::bind_method(D_METHOD("set_video_codec", "codec"), &MoonlightStream::set_video_codec);
    ClassDB::bind_method(D_METHOD("get_video_codec"), &MoonlightStream::get_video_codec);
    ClassDB::bind_method(D_METHOD("set_color_space", "color_space"), &MoonlightStream::set_color_space);
    ClassDB::bind_method(D_METHOD("get_color_space"), &MoonlightStream::get_color_space);
    ClassDB::bind_method(D_METHOD("set_enable_hdr", "enable"), &MoonlightStream::set_enable_hdr);
    ClassDB::bind_method(D_METHOD("get_enable_hdr"), &MoonlightStream::get_enable_hdr);
    ClassDB::bind_method(D_METHOD("get_audio_stream"), &MoonlightStream::get_audio_stream);
    
    ClassDB::bind_method(D_METHOD("start_connection"), &MoonlightStream::start_connection);
    ClassDB::bind_method(D_METHOD("stop_connection"), &MoonlightStream::stop_connection);
    
    ClassDB::bind_method(D_METHOD("send_mouse_button_event", "button", "pressed"), &MoonlightStream::send_mouse_button_event);
    ClassDB::bind_method(D_METHOD("send_mouse_move_event", "x", "y"), &MoonlightStream::send_mouse_move_event);
    ClassDB::bind_method(D_METHOD("send_key_event", "scancode", "pressed"), &MoonlightStream::send_key_event);
    ClassDB::bind_method(D_METHOD("set_remote_input_aes_key", "key"), &MoonlightStream::set_remote_input_aes_key);
    ClassDB::bind_method(D_METHOD("set_remote_input_aes_iv", "iv"), &MoonlightStream::set_remote_input_aes_iv);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "host_address"), "set_host_address", "get_host_address");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "app_id"), "set_app_id", "get_app_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_app_version"), "set_server_app_version", "get_server_app_version");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "server_codec_mode_support"), "set_server_codec_mode_support", "get_server_codec_mode_support");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "server_rtsp_session_url"), "set_server_rtsp_session_url", "get_server_rtsp_session_url");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "fps", PROPERTY_HINT_RANGE, "10,120,1"), "set_fps", "get_fps");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "bitrate_kbps", PROPERTY_HINT_RANGE, "500,100000,1"), "set_bitrate_kbps", "get_bitrate_kbps");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "video_codec", PROPERTY_HINT_ENUM, "H264,HEVC,AV1"), "set_video_codec", "get_video_codec");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "color_space", PROPERTY_HINT_ENUM, "Rec601,Rec709,Rec2020"), "set_color_space", "get_color_space");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enable_hdr"), "set_enable_hdr", "get_enable_hdr");

    BIND_ENUM_CONSTANT(CODEC_H264);
    BIND_ENUM_CONSTANT(CODEC_HEVC);
    BIND_ENUM_CONSTANT(CODEC_AV1);
    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_601);
    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_709);
    BIND_ENUM_CONSTANT(COLOR_SPACE_REC_2020);

    // === 注册信号 ===
    ADD_SIGNAL(MethodInfo("connection_started"));
    ADD_SIGNAL(MethodInfo("connection_terminated", PropertyInfo(Variant::INT, "error_code")));
    ADD_SIGNAL(MethodInfo("connection_failed", PropertyInfo(Variant::STRING, "message")));
}

// ============ Lifecycle ============
MoonlightStream::MoonlightStream() {
    void* render_context = this;
    instance_map[render_context] = this;
    
    // 在主线程提前初始化 AudioStream，避免在回调线程中创建对象的风险
    audio_stream.instantiate();
    audio_stream->set_mix_rate(48000);

    viewport = memnew(SubViewport);
    viewport->set_name("VideoViewport");
    viewport->set_size(Size2i(width, height));
    viewport->set_disable_3d(true);
    viewport->set_clear_mode(SubViewport::CLEAR_MODE_ALWAYS);
    viewport->set_transparent_background(false);
    viewport->set_update_mode(SubViewport::UPDATE_ALWAYS);
    add_child(viewport);
    texture_rid = RenderingServer::get_singleton()->viewport_get_texture(viewport->get_viewport_rid());
}

MoonlightStream::~MoonlightStream() {
    instance_map.erase(this);
    
    // 如果是当前全局流，清除指针
    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }

    stop_connection(); // 确保线程 join
    
    if (viewport) {
        viewport->queue_free();
        viewport = nullptr;
    }
}

void MoonlightStream::set_resolution(int p_width, int p_height) {
    width = p_width;
    height = p_height;
    if (viewport) {
        viewport->set_size(Size2i(width, height));
    }
}

// ============ Connection Control ============

void MoonlightStream::start_connection() {
    // 检查是否已在运行
    if (streaming.load()) {
        UtilityFunctions::push_warning("MoonlightStream: Already streaming.");
        return;
    }
    
    // 如果之前的线程还没彻底退出，等待它
    if (connection_thread.joinable()) {
        connection_thread.join();
    }

    // === 参数校验 ===
    if (custom_aes_key.is_empty() || custom_aes_key.size() != 16) {
        emit_signal("connection_failed", "Invalid AES Key (must be 16 bytes)");
        return;
    }
    if (custom_aes_iv.is_empty() || custom_aes_iv.size() < 4) {
        emit_signal("connection_failed", "Invalid AES IV (must be >= 4 bytes)");
        return;
    }
    if (server_app_version.is_empty()) {
        emit_signal("connection_failed", "Server App Version not set");
        return;
    }

    // 设置全局活跃流指针 (供 C 回调使用)
    global_active_stream = this;
    streaming.store(true);

    // 启动后台线程
    connection_thread = std::thread(&MoonlightStream::_connection_thread_func, this);
}

void MoonlightStream::_connection_thread_func() {
    // 在线程栈上准备数据，保证 LiStartConnection 执行期间数据有效
    CharString host_str = host_address.utf8();
    CharString app_ver_str = server_app_version.utf8();
    CharString rtsp_url_str = server_rtsp_session_url.utf8();

    SERVER_INFORMATION server_info = {};
    server_info.address = host_str.get_data();
    server_info.serverInfoAppVersion = app_ver_str.get_data();
    server_info.serverCodecModeSupport = server_codec_mode_support;
    if (!server_rtsp_session_url.is_empty()) {
        server_info.rtspSessionUrl = rtsp_url_str.get_data();
    } else {
        server_info.rtspSessionUrl = nullptr;
    }

    STREAM_CONFIGURATION stream_config = {};
    stream_config.width = width;
    stream_config.height = height;
    stream_config.fps = fps;
    stream_config.bitrate = bitrate_kbps;
    stream_config.packetSize = 1392;
    stream_config.streamingRemotely = STREAM_CFG_LOCAL;
    stream_config.audioConfiguration = AUDIO_CONFIGURATION_STEREO;
    stream_config.supportedVideoFormats = static_cast<int>(video_codec);
    stream_config.colorSpace = static_cast<int>(color_space);
    stream_config.colorRange = enable_hdr ? COLOR_RANGE_FULL : COLOR_RANGE_LIMITED;

    // 填充 AES 密钥
    memcpy(stream_config.remoteInputAesKey, custom_aes_key.ptr(), 16);
    memcpy(stream_config.remoteInputAesIv, custom_aes_iv.ptr(), 4);
    memset(stream_config.remoteInputAesIv + 4, 0, 12);

    // 准备回调
    DECODER_RENDERER_CALLBACKS dr_callbacks = {};
    dr_callbacks.setup = dr_setup;
    dr_callbacks.submitDecodeUnit = dr_submit_decode_unit;
    dr_callbacks.cleanup = dr_cleanup;
    dr_callbacks.capabilities = CAPABILITY_DIRECT_SUBMIT;

    AUDIO_RENDERER_CALLBACKS ar_callbacks = {};
    ar_callbacks.init = ar_init;
    ar_callbacks.decodeAndPlaySample = ar_decode_and_play_sample;
    ar_callbacks.capabilities = CAPABILITY_DIRECT_SUBMIT;

    CONNECTION_LISTENER_CALLBACKS cl_callbacks = {};
    // 由于 C 回调签名不带 context，我们使用 lambda 并访问全局静态指针
    cl_callbacks.stageStarting = [](int stage) { 
        UtilityFunctions::print("Moonlight Stage Starting: ", stage); 
    };
    cl_callbacks.stageFailed = [](int stage, int error) {
        UtilityFunctions::push_warning(vformat("Moonlight Stage %d Failed: %d", stage, error));
    };
    cl_callbacks.connectionStarted = []() {
        UtilityFunctions::print("Moonlight Connection Started!");
        if (global_active_stream) {
            global_active_stream->_emit_connection_started();
        }
    };
    cl_callbacks.connectionTerminated = [](int error) {
        UtilityFunctions::print("Moonlight Connection Terminated: ", error);
        // 此时 LiStartConnection 可能还没返回，我们在线程结束处统一发信号
    };
    // Log 也可以加，看需求

    void* render_context = this; // 传给 video/audio callback 的 context
    
    // === 阻塞调用 ===
    int ret = LiStartConnection(
        &server_info,
        &stream_config,
        &cl_callbacks,
        &dr_callbacks,
        &ar_callbacks,
        render_context,
        0,
        render_context, // Audio context
        0
    );

    // 连接结束
    streaming.store(false);
    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }

    // 通知主线程连接已结束
    call_deferred("_emit_connection_terminated", ret);
}

void MoonlightStream::stop_connection() {
    // 1. 发出停止指令，打破阻塞
    if (streaming.load()) {
        LiStopConnection();
    }

    // 2. 等待线程完全退出
    if (connection_thread.joinable()) {
        connection_thread.join();
    }
    
    // 3. 状态清理
    streaming.store(false);
    if (global_active_stream == this) {
        global_active_stream = nullptr;
    }
}

// ============ Signal Helpers (run on Main Thread via call_deferred) ============
void MoonlightStream::_emit_connection_started() {
    // 再次通过 call_deferred 确保万无一失（虽然从 lambda 调过来已经是 deferred）
    call_deferred("emit_signal", "connection_started");
}

void MoonlightStream::_emit_connection_terminated(int error_code) {
    call_deferred("emit_signal", "connection_terminated", error_code);
}

// ============ Frame Rendering ============
int MoonlightStream::_on_submit_decode_unit(PDECODE_UNIT decode_unit) {
    if (!streaming.load() || width <= 0 || height <= 0 || (width & 1) || (height & 1)) {
        return DR_OK;
    }

    size_t y_size = width * height;
    size_t uv_size = (width / 2) * (height / 2);
    size_t total_expected = y_size + uv_size * 2;
    uint8_t* full_buffer = new uint8_t[total_expected];
    size_t offset = 0;
    PLENTRY entry = decode_unit->bufferList;
    while (entry && offset < total_expected) {
        size_t len = (offset + entry->length <= total_expected) ? entry->length : (total_expected - offset);
        std::memcpy(full_buffer + offset, entry->data, len);
        offset += entry->length;
        entry = entry->next;
    }

    if (offset < total_expected) {
        delete[] full_buffer;
        return DR_OK;
    }

    const uint8_t* y = full_buffer;
    const uint8_t* u = full_buffer + y_size;
    const uint8_t* v = full_buffer + y_size + uv_size;
    size_t rgba_size = width * height * 4;
    uint8_t* rgba = new uint8_t[rgba_size];
    yuv420p_to_rgba(y, u, v, rgba, width, height);

    PackedByteArray image_data;
    image_data.resize(rgba_size);
    memcpy(image_data.ptrw(), rgba, rgba_size);

    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        pending_frame = image_data;
        has_pending_frame = true;
    }

    delete[] rgba;
    delete[] full_buffer;
    return DR_OK;
}

// ============ Main Thread Process ============
void MoonlightStream::_process(double delta) {
    PackedByteArray frame;
    bool update = false;
    {
        std::lock_guard<std::mutex> lock(frame_mutex);
        if (has_pending_frame) {
            frame = pending_frame;
            has_pending_frame = false;
            update = true;
        }
    }

    if (update) {
        Ref<Image> image = Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, frame);
        RenderingServer::get_singleton()->texture_2d_update(texture_rid, image, 0);
    }
}

// ============ Input ============
void MoonlightStream::send_mouse_button_event(int button, bool pressed) {
    if (streaming.load()) LiSendMouseButtonEvent(pressed ? BUTTON_ACTION_PRESS : BUTTON_ACTION_RELEASE, button);
}

void MoonlightStream::send_mouse_move_event(float x, float y) {
    if (streaming.load()) LiSendMouseMoveEvent(static_cast<short>(x), static_cast<short>(y));
}

void MoonlightStream::send_key_event(int scancode, bool pressed) {
    if (streaming.load()) LiSendKeyboardEvent(static_cast<short>(scancode), pressed ? KEY_ACTION_DOWN : KEY_ACTION_UP, 0);
}

void MoonlightStream::set_remote_input_aes_key(const PackedByteArray &p_key) {
    custom_aes_key = p_key;
}

void MoonlightStream::set_remote_input_aes_iv(const PackedByteArray &p_iv) {
    custom_aes_iv = p_iv;
}