class_name MoonlightConnector
extends Node

var _streamer: Node = null # MoonlightStream GDExtension

func start_stream(parent: Node, host: String, app_id: int, config: Dictionary, aes_key: PackedByteArray, aes_iv: PackedByteArray):
    if not ClassDB.class_exists("MoonlightStream"):
        printerr("GDExtension missing")
        return
        
    stop_stream()
    
    _streamer = ClassDB.instantiate("MoonlightStream")
    parent.add_child(_streamer)
    
    # 设置基础属性
    _streamer.set_host_address(host)
    _streamer.set_app_id(app_id)
    _streamer.set_server_app_version(config.get("app_version", "1.0.0"))
    _streamer.set_server_codec_mode_support(config.get("codec_mode", 0))
    _streamer.set_server_rtsp_session_url(config.get("rtsp_url", ""))
    
    # 设置流参数
    _streamer.set_resolution(config.get("width", 1280), config.get("height", 720))
    _streamer.set_fps(config.get("fps", 60))
    _streamer.set_bitrate_kbps(config.get("bitrate", 10000))
    
    # 设置 AES 密钥 (来自 /launch 响应)
    _streamer.set_remote_input_aes_key(aes_key)
    _streamer.set_remote_input_aes_iv(aes_iv)
    
    # 绑定信号
    _streamer.connect("connection_started", func(): print("Stream Started"))
    _streamer.connect("connection_stopped", func(code): print("Stream Stopped: ", code))
    
    # 启动 (异步)
    _streamer.start_connection()

func stop_stream():
    if _streamer:
        _streamer.stop_connection()
        _streamer.queue_free()
        _streamer = null
