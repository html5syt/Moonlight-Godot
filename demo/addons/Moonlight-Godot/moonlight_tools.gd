class_name MoonlightTools
extends Node

# --- 路径常量 ---
const CLIENT_CERT_PATH = "user://client.crt"
const CLIENT_KEY_PATH = "user://client.key"
const SERVER_CERT_PATH = "user://server.crt"

# --- 运行配置 ---
var server_ip: String = "127.0.0.1"
var http_port: int = 47989
var https_port: int = 47984
var unique_id: String = ""

# --- 内部实例 ---
var _crypto: Crypto = Crypto.new()

func _ready():
    if unique_id.is_empty():
        unique_id = _get_or_create_uuid()
    _ensure_client_certs()

# --- 证书管理 ---

func _ensure_client_certs():
    if FileAccess.file_exists(CLIENT_CERT_PATH) and FileAccess.file_exists(CLIENT_KEY_PATH):
        return

    print("MoonlightTools: 生成客户端 RSA 密钥与证书...")
    var key = _crypto.generate_rsa(2048)
    var cert = _crypto.generate_self_signed_certificate(
        key, 
        "CN=NVIDIA GameStream Client,O=GodotMoonlight,C=US", 
        "20200101000000", 
		"20400101000000"
    )
    key.save(CLIENT_KEY_PATH)
    cert.save(CLIENT_CERT_PATH)

func get_client_cert_string() -> String:
    if not FileAccess.file_exists(CLIENT_CERT_PATH): return ""
    var f = FileAccess.open(CLIENT_CERT_PATH, FileAccess.READ)
    var text = f.get_as_text()
    text = text.replace("-----BEGIN CERTIFICATE-----", "")
    text = text.replace("-----END CERTIFICATE-----", "")
    text = text.replace("\n", "").replace("\r", "")
    return text

# --- 网络请求 ---

## 获取 TLS 配置 (核心修改)
## verify_server: 是否需要验证服务器证书 (Pinned Certificate)
## use_client_auth: 是否需要发送客户端证书 (mTLS)
func get_tls_options(verify_server: bool = true, use_client_auth: bool = true) -> TLSOptions:
    var trusted_cas = X509Certificate.new()
    var has_server_cert = false
    
    # 1. 加载服务器证书 (用于验证对方)
    if verify_server and FileAccess.file_exists(SERVER_CERT_PATH):
        if trusted_cas.load(SERVER_CERT_PATH) == OK:
            has_server_cert = true

    # 2. 加载客户端证书 (用于表明自己身份)
    var client_cert = X509Certificate.new()
    var client_key = CryptoKey.new()
    var has_client_identity = false
    
    if use_client_auth and FileAccess.file_exists(CLIENT_CERT_PATH) and FileAccess.file_exists(CLIENT_KEY_PATH):
        if client_cert.load(CLIENT_CERT_PATH) == OK and client_key.load(CLIENT_KEY_PATH) == OK:
            has_client_identity = true

    # --- 配置生成逻辑 ---
    
    # 如果需要携带客户端证书进行请求 (mTLS)
    # 根据指示：使用 .server() 配置来携带客户端证书
    if has_client_identity:
        # 注意：TLSOptions.server(key, cert) 创建包含私钥和证书的配置
        # 在发起请求时使用此配置，即可在握手阶段发送客户端证书
        return TLSOptions.server(client_key, client_cert)
    
    # 如果不需要客户端证书，仅验证服务器 (单向验证)
    if verify_server and has_server_cert:
        return TLSOptions.client(trusted_cas)
    
    # 既无客户端证书，也不强制验证服务器 (不安全/初始配对阶段)
    return TLSOptions.client_unsafe(trusted_cas if has_server_cert else null)

## 发送 HTTP 请求
## use_client_auth: 控制是否尝试使用 mTLS
func send_request(parent: Node, path: String, use_https: bool = false, use_client_auth: bool = true) -> Dictionary:
    var req = HTTPRequest.new()
    parent.add_child(req)
    
    var port = https_port if use_https else http_port
    var protocol = "https" if use_https else "http"
    var url = "%s://%s:%d%s" % [protocol, server_ip, port, path]
    
    if use_https:
        # 传递 use_client_auth 参数
        req.set_tls_options(get_tls_options(true, use_client_auth))
    
    req.request(url)
    var result = await req.request_completed
    req.queue_free()
    
    if result[0] != HTTPRequest.RESULT_SUCCESS:
        return {"success": false, "code": 0, "body": ""}
        
    return {
        "success": true, 
        "code": result[1], 
        "body": result[3].get_string_from_utf8(),
        "raw_body": result[3]
    }

# --- 数据处理辅助 ---

func _get_or_create_uuid() -> String:
    var rng = RandomNumberGenerator.new()
    rng.randomize()
    var uuid = ""
    var chars = "0123456789abcdef"
    for i in range(16):
        uuid += chars[rng.randi() % 16]
    return uuid

func parse_xml(xml_str: String) -> Dictionary:
    var parser = XMLParser.new()
    parser.open_buffer(xml_str.to_utf8_buffer())
    var dict = {}
    var current_tag = ""
    while parser.read() != ERR_FILE_EOF:
        if parser.get_node_type() == XMLParser.NODE_ELEMENT:
            current_tag = parser.get_node_name()
        elif parser.get_node_type() == XMLParser.NODE_TEXT:
            if current_tag != "":
                dict[current_tag] = parser.get_node_data()
    return dict
