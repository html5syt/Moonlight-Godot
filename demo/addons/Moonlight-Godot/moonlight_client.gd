class_name MoonlightClient
extends Node

# ... (信号定义) ...

var _tools: MoonlightTools
var _pairing: MoonlightPairing
var _connector: MoonlightConnector
var _server_config_cache = {} # 缓存 serverinfo

@export var server_ip: String = "192.168.1.100"

func _ready():
    _tools = MoonlightTools.new()
    _tools.server_ip = server_ip
    add_child(_tools)
    
    _pairing = MoonlightPairing.new(_tools)
    add_child(_pairing)
    
    _connector = MoonlightConnector.new() # 这里不需要 add_child，因为它是辅助类
    
    # 自动获取一次 serverinfo 以确定 HTTP/HTTPS 端口和版本
    fetch_server_info()

func fetch_server_info():
    # 优先尝试 HTTPS (Pinning)，失败回退 HTTP
    var res = await _tools.request(self, "/serverinfo", 1)
    if not res.success:
        res = await _tools.request(self, "/serverinfo", 0)
    
    if res.success:
        var xml = _tools.parse_xml(res.body)
        _server_config_cache = xml # 缓存 appversion, servercodecmodesupport 等
        emit_signal("server_found", xml)

func start_pairing():
    _pairing.start()

func launch_app(app_id: int):
    # 1. 生成密钥 (rikey/rikeyid)
    var crypto = Crypto.new()
    var aes_key = crypto.generate_random_bytes(16)
    var aes_iv = crypto.generate_random_bytes(16) # 前4字节做 ID
    
    var key_hex = aes_key.hex_encode()
    var id_int = aes_iv.decode_u32(0) # 取前4字节转 int
    
    # 2. 构造 /launch URL (必须 HTTPS mTLS)
    var width = 1280
    var height = 720
    var fps = 60
    
    var url = "/launch?uniqueid=%s&appid=%d&mode=%dx%dx%d&rikey=%s&rikeyid=%d" \
        % [_tools.unique_id, app_id, width, height, fps, key_hex, id_int]
    
    # 3. 发送请求
    var res = await _tools.request(self, url, 2) # mTLS
    
    if res.success and res.code == 200:
        var xml = _tools.parse_xml(res.body)
        var rtsp_url = xml.get("sessionUrl0", "")
        
        # 4. 启动连接
        var config = {
            "width": width, "height": height, "fps": fps, "bitrate": 10000,
            "rtsp_url": rtsp_url,
            "app_version": _server_config_cache.get("appversion", ""),
            "codec_mode": _server_config_cache.get("ServerCodecModeSupport", "0").to_int()
        }
        
        _connector.start_stream(self, server_ip, app_id, config, aes_key, aes_iv)
    else:
        printerr("Launch failed: ", res.code)
