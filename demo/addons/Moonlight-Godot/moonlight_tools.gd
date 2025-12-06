class_name MoonlightTools
extends Node

# 路径配置
const CERT_DIR = "user://certs"
const CLIENT_CERT_PATH = CERT_DIR + "/client.crt"
const CLIENT_KEY_PATH = CERT_DIR + "/client.key"
const SERVER_CERT_PATH = CERT_DIR + "/server.pem"

var server_ip: String = ""
var http_port: int = 47989
var https_port: int = 47984
var unique_id: String = "0123456789ABCDEF" # 固定 ID

func _ready():
    if not DirAccess.dir_exists_absolute(CERT_DIR):
        DirAccess.make_dir_absolute(CERT_DIR)
    _ensure_identity()

# --- 身份管理 (IdentityManager.cpp) ---
func _ensure_identity():
    if FileAccess.file_exists(CLIENT_CERT_PATH) and FileAccess.file_exists(CLIENT_KEY_PATH):
        return
    
    print("Generating new client identity...")
    var crypto = Crypto.new()
    var key = crypto.generate_rsa(2048)
    var cert = crypto.generate_self_signed_certificate(key, "CN=NVIDIA GameStream Client", "20200101000000", "20400101000000")
    key.save(CLIENT_KEY_PATH)
    cert.save(CLIENT_CERT_PATH)

func get_client_cert_hex() -> String:
    var f = FileAccess.open(CLIENT_CERT_PATH, FileAccess.READ)
    return f.get_as_text().to_utf8_buffer().hex_encode()

func save_server_cert(hex_data: String):
    var bytes = hex_data.hex_decode() # Qt 源码中 serverCertStr 是 hex decode 后的 PEM
    var f = FileAccess.open(SERVER_CERT_PATH, FileAccess.WRITE)
    f.store_buffer(bytes)

# --- TLS 配置 (参考 nvhttp.cpp setSslConfiguration) ---
# 模式 1: Pinning (验证服务器证书) - 用于 /serverinfo 和 HTTPS 握手
func get_tls_pinning() -> TLSOptions:
    if FileAccess.file_exists(SERVER_CERT_PATH):
        var cert = X509Certificate.new()
        if cert.load(SERVER_CERT_PATH) == OK:
            return TLSOptions.client_unsafe(cert) # 校验 CA 但忽略 Hostname (IP 访问常见问题)
    return TLSOptions.client_unsafe()

# 模式 2: mTLS (携带客户端证书) - 用于 /launch, /pair (stage 5)
func get_tls_mtls() -> TLSOptions:
    var key = CryptoKey.new()
    var cert = X509Certificate.new()
    if key.load(CLIENT_KEY_PATH) == OK and cert.load(CLIENT_CERT_PATH) == OK:
        # 注意：Godot 的 client() 目前不支持直接传 client cert。
        # 这里的 trick 是使用 server() 配置，它允许指定 key/cert。
        # 虽然命名是 server，但底层 SSL context 只要配了 key/cert，在握手请求 client cert 时就会发送。
        return TLSOptions.server(key, cert)
    return null

# --- HTTP 请求 ---
# tls_mode: 0=HTTP, 1=HTTPS(Pinning), 2=HTTPS(mTLS)
func request(parent: Node, path: String, tls_mode: int = 0) -> Dictionary:
    var req = HTTPRequest.new()
    parent.add_child(req)
    req.timeout = 5.0 # FAST_FAIL_TIMEOUT_MS
    
    var port = https_port if tls_mode > 0 else http_port
    var proto = "https" if tls_mode > 0 else "http"
    var url = "%s://%s:%d%s" % [proto, server_ip, port, path]
    
    if tls_mode == 1:
        req.set_tls_options(get_tls_pinning())
    elif tls_mode == 2:
        var mtls = get_tls_mtls()
        if mtls: req.set_tls_options(mtls)
    
    req.request(url)
    var res = await req.request_completed
    req.queue_free()
    
    # [Result, Code, Headers, Body]
    if res[0] != HTTPRequest.RESULT_SUCCESS:
        return {"success": false, "code": 0, "body": ""}
    
    return {
        "success": true,
        "code": res[1],
        "body": res[3].get_string_from_utf8()
    }

func parse_xml(xml: String) -> Dictionary:
    var p = XMLParser.new()
    p.open_buffer(xml.to_utf8_buffer())
    var d = {}
    var tag = ""
    while p.read() == OK:
        if p.get_node_type() == XMLParser.NODE_ELEMENT:
            tag = p.get_node_name()
        elif p.get_node_type() == XMLParser.NODE_TEXT:
            if tag != "": d[tag] = p.get_node_data()
    return d
