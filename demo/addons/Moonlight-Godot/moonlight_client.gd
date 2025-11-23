class_name MoonlightClient
extends Node

# --- 信号 ---
signal server_info_received(info: Dictionary)
signal app_list_received(apps: Array)
signal app_icon_received(app_id: String, texture: ImageTexture)
signal pairing_pin(pin: String)
signal pairing_message(msg: String)
signal pairing_success
signal pairing_failed(err: String)

# --- 导出变量 ---
@export var server_ip: String = "127.0.0.1":
    set(val):
        server_ip = val
        if _tools: _tools.server_ip = val

# --- 模块实例 ---
var _tools: MoonlightTools
var _pairing: MoonlightPairing
var _connector: MoonlightConnector

func _ready():
    # 1. 初始化工具
    _tools = MoonlightTools.new()
    _tools.server_ip = server_ip
    add_child(_tools)
    
    # 2. 初始化配对逻辑
    _pairing = MoonlightPairing.new(_tools)
    add_child(_pairing)
    
    # 3. 初始化连接器
    _connector = MoonlightConnector.new(_tools)
    add_child(_connector)
    
    # 4. 连接内部信号
    _pairing.pairing_pin_generated.connect(func(p): emit_signal("pairing_pin", p))
    _pairing.pairing_status_changed.connect(func(s, m): emit_signal("pairing_message", m))
    _pairing.pairing_success.connect(func(): emit_signal("pairing_success"))
    _pairing.pairing_failed.connect(func(e): emit_signal("pairing_failed", e))

# --- 公开 API ---

## 获取服务器信息
func fetch_server_info():
    var res = await _tools.send_request(self, "/serverinfo")
    if res.success:
        var info = _tools.parse_xml(res.body)
        emit_signal("server_info_received", info)
    else:
        printerr("Server info fetch failed.")

## 开始配对
func start_pairing():
    _pairing.start_pairing()

## 取消配对
func cancel_pairing():
    _pairing.cancel_pairing()

## 获取应用列表 (HTTPS)
func fetch_app_list():
    # 注意：此步骤在原生 Godot 中可能会失败，因为缺少 mTLS 支持
    var url = "/applist?uniqueid=%s" % _tools.unique_id
    var res = await _tools.send_request(self, url, true) # Use HTTPS
    
    if res.success:
        var apps = []
        var parser = XMLParser.new()
        parser.open_buffer(res.raw_body)
        var current_app = {}
        var in_app = false
        var current_tag = ""
        
        while parser.read() != ERR_FILE_EOF:
            if parser.get_node_type() == XMLParser.NODE_ELEMENT:
                if parser.get_node_name() == "App":
                    in_app = true
                    current_app = {}
                current_tag = parser.get_node_name()
            elif parser.get_node_type() == XMLParser.NODE_TEXT and in_app:
                current_app[current_tag] = parser.get_node_data()
            elif parser.get_node_type() == XMLParser.NODE_ELEMENT_END:
                if parser.get_node_name() == "App":
                    in_app = false
                    apps.append(current_app)
        
        emit_signal("app_list_received", apps)
    else:
        printerr("应用列表获取失败 (可能是由于 mTLS 不支持): Code ", res.code)

## 获取图标
func fetch_app_icon(app_id: String):
    var req = HTTPRequest.new()
    add_child(req)
    req.set_tls_options(_tools.get_tls_options(true))
    
    var url = "https://%s:%d/appicon?uniqueid=%s&appid=%s" % [server_ip, _tools.https_port, _tools.unique_id, app_id]
    req.request(url)
    
    var result = await req.request_completed
    req.queue_free()
    
    if result[1] == 200:
        var img = Image.new()
        # 尝试加载 PNG
        var err = img.load_png_from_buffer(result[3])
        if err == OK:
            emit_signal("app_icon_received", app_id, ImageTexture.create_from_image(img))

## 开始串流
func start_stream(display_container: Node, app_id: String, options: Dictionary = {}):
    _connector.start_session(display_container, app_id, options)

## 停止串流
func stop_stream():
    _connector.stop_session()
