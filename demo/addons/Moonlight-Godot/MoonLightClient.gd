extends Node
class_name MoonLightClient

## 配置常量
const CLIENT_CERT_PATH = "user://addons/moonlight-godot/client.pem"
const CLIENT_KEY_PATH = "user://addons/moonlight-godot/client.key"
const DEFAULT_SERVER_PORT = 47984
const DEFAULT_APP_VERSION = "3.20.3.70"

## 配对状态
enum PairingState {NOT_STARTED, IN_PROGRESS, SUCCESS, FAILED, CERT_GENERATED}

## 连接状态
enum ConnectionState {DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING}

## 服务端状态
enum ServerState {SERVER_READY, SERVER_BUSY, SERVER_OFFLINE}

## 应用信息
class AppInfo:
    var title: String = ""
    var id: int = 0
    var is_hdr_supported: bool = false
    var is_app_collector_game: bool = false

## 会话信息
class SessionInfo:
    var session_url: String = ""
    var udp_port: int = 0
    var tcp_port: int = 0

## 服务端信息
class ServerInfo:
    var hostname: String = ""
    var https_port: int = DEFAULT_SERVER_PORT
    var state: ServerState = ServerState.SERVER_READY
    var current_game: int = 0
    var gfe_version: String = ""
    var pair_status: int = 0

## 内部状态
var pairing_state: PairingState = PairingState.NOT_STARTED
var connection_state: ConnectionState = ConnectionState.DISCONNECTED
var server_info: ServerInfo = ServerInfo.new()

## 证书与密钥
var client_cert: String = ""
var client_key: String = ""

## 应用列表等
@export var app_list: Array = []
var moonlight_stream: MoonlightStream = null
var current_session: SessionInfo = SessionInfo.new()
@export var current_app_id: int = 0
var current_app_asset: Texture = null
var error_message: String = ""
var pairing_progress: int = 0
var pairing_pin: String = ""
var pairing_salt: PackedByteArray = []
var pairing_aes_key: PackedByteArray = []
var pairing_client_secret: PackedByteArray = []
var pairing_server_signature: PackedByteArray = []
var pairing_server_cert: String = ""
var pairing_server_response: String = ""
var pairing_client_response: String = ""
var pairing_challenge: PackedByteArray = []
var pairing_challenge_response: PackedByteArray = []
var pairing_pairing_secret: PackedByteArray = []
var pairing_client_pairing_secret: PackedByteArray = []
var pairing_final_response: String = ""
var pairing_final_state: bool = false

@export var server_ip: String = "localhost"

func _init():
    var dir = DirAccess.open("user://addons/moonlight-godot")
    if not dir:
        dir = DirAccess.open("user://")
        dir.make_dir_recursive("user://addons/moonlight-godot")
    _load_client_certificate()
    pairing_state = PairingState.NOT_STARTED
    connection_state = ConnectionState.DISCONNECTED

func _load_client_certificate() -> void:
    var cert = X509Certificate.new()
    var key = CryptoKey.new()
    if cert.load(CLIENT_CERT_PATH) != OK or key.load(CLIENT_KEY_PATH) != OK:
        _generate_client_certificate()
        return
    client_cert = cert.save_to_string()
    client_key = key.save_to_string()
    pairing_state = PairingState.CERT_GENERATED
    error_message = "Loaded existing client certificate"

func _generate_client_certificate() -> void:
    var crypto = Crypto.new()
    var key = crypto.generate_rsa(2048)
    var cert = crypto.generate_self_signed_certificate(key, "CN=NVIDIA GameStream Client")
    _save_certificate(cert, key)

func _save_certificate(cert: X509Certificate, key: CryptoKey) -> void:
    if cert.save(CLIENT_CERT_PATH) != OK or key.save(CLIENT_KEY_PATH) != OK:
        error_message = "Failed to save certificate or key"
        return
    pairing_state = PairingState.CERT_GENERATED
    error_message = "Certificate and key saved successfully"

# ==================== 配对流程（修正版） ====================

func pair_with_server(pin: String) -> bool:
    if pairing_state == PairingState.SUCCESS:
        error_message = "Already paired with server"
        return false
    pairing_pin = pin
    pairing_state = PairingState.IN_PROGRESS
    pairing_progress = 0

    if not await _pairing_stage_1():
        error_message = "Failed to get server certificate"
        pairing_state = PairingState.FAILED
        return false

    if not await _pairing_stage_2():
        error_message = "Failed to send client challenge"
        pairing_state = PairingState.FAILED
        return false

    if not await _pairing_stage_3():
        error_message = "Failed to respond to server challenge"
        pairing_state = PairingState.FAILED
        return false

    if not await _pairing_stage_4():
        error_message = "Failed to submit client pairing secret"
        pairing_state = PairingState.FAILED
        return false

    var attempts = 0
    while not await _pairing_stage_5():
        attempts += 1
        if attempts > 60:
            error_message = "Pairing timeout"
            pairing_state = PairingState.FAILED
            return false
        await get_tree().create_timer(1.0).timeout

    pairing_state = PairingState.SUCCESS
    error_message = "Pairing successful"
    return true

func _http_get(url: String) -> Dictionary:
    var http = HTTPRequest.new()
    add_child(http)
    var err = http.request(url)
    if err != OK:
        http.queue_free()
        return {"success": false, "error": str(err)}
    await http.request_completed
    var result = {
        "success": http.get_response_code() == 200,
        "code": http.get_response_code(),
        "body": http.get_response_body_as_string()
    }
    http.queue_free()
    return result

func _pairing_stage_1() -> bool:
    pairing_progress = 1
    pairing_salt = Crypto.new().generate_random_bytes(16)
    var salt_b64 = Marshalls.raw_to_base64(pairing_salt)
    var cert_b64 = Marshalls.utf8_to_base64(client_cert)
    var url = "http://%s:%d/pair?phrase=getservercert&devicename=roth&updateState=1&salt=%s&clientcert=%s" % [server_ip, DEFAULT_SERVER_PORT, salt_b64, cert_b64]
    
    var resp = await _http_get(url)
    if not resp.success:
        return false

    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return false

    var root = xml.get_node()
    if root.get_child_count() < 2:
        return false
    if root.get_child(0).get_text() != "1":
        return false
    pairing_server_cert = root.get_child(1).get_text()
    return true

func _derive_aes_key_from_pin(pin: String, salt: PackedByteArray) -> PackedByteArray:
    var hasher = HashingContext.new()
    hasher.start(HashingContext.HASH_SHA256)
    hasher.update((pin).to_utf8_buffer())
    hasher.update(salt)
    var hash = hasher.finish()
    return hash.slice(0, 16)  # AES-128 key

func _encrypt_ecb(data: PackedByteArray, key: PackedByteArray) -> PackedByteArray:
    # 验证密钥长度：必须为16或32字节
    assert(key.size() == 16 or key.size() == 32, "Key must be 16 or 32 bytes for AES-128/AES-256.")
    # 验证数据长度：必须为16的倍数（ECB模式要求）
    assert(data.size() % 16 == 0, "Data length must be a multiple of 16 bytes in ECB mode.")

    var aes = AESContext.new()
    var err = aes.start(AESContext.MODE_ECB_ENCRYPT, key)
    if err != OK:
        push_error("Failed to start AES ECB encryption.")
        return PackedByteArray()

    var encrypted = aes.update(data)
    aes.finish()
    return encrypted


func _decrypt_ecb(data: PackedByteArray, key: PackedByteArray) -> PackedByteArray:
    # 同样验证密钥和数据长度
    assert(key.size() == 16 or key.size() == 32, "Key must be 16 or 32 bytes.")
    assert(data.size() % 16 == 0, "Encrypted data length must be a multiple of 16 bytes.")

    var aes = AESContext.new()
    var err = aes.start(AESContext.MODE_ECB_DECRYPT, key)
    if err != OK:
        push_error("Failed to start AES ECB decryption.")
        return PackedByteArray()

    var decrypted = aes.update(data)
    aes.finish()
    return decrypted

func _pairing_stage_2() -> bool:
    pairing_progress = 2
    pairing_aes_key = _derive_aes_key_from_pin(pairing_pin, pairing_salt)
    pairing_challenge = Crypto.new().generate_random_bytes(16)
    var encrypted = _encrypt_ecb(pairing_challenge, pairing_aes_key)
    var enc_b64 = Marshalls.raw_to_base64(encrypted)
    var url = "http://%s:%d/pair?devicename=roth&updateState=1&clientchallenge=%s" % [server_ip, DEFAULT_SERVER_PORT, enc_b64]

    var resp = await _http_get(url)
    if not resp.success:
        return false

    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return false

    var root = xml.get_node()
    if root.get_child(0).get_text() != "1":
        return false
    pairing_challenge_response = Marshalls.base64_to_raw(root.get_child(1).get_text())
    return true

func _pairing_stage_3() -> bool:
    pairing_progress = 3
    var decrypted = _decrypt_ecb(pairing_challenge_response, pairing_aes_key)
    if len(decrypted) < 48:
        return false
    var server_challenge = decrypted.slice(0, 16)
    var server_signature = decrypted.slice(16, 48)

    pairing_client_secret = Crypto.new().generate_random_bytes(16)
    var hasher = HashingContext.new()
    hasher.start(HashingContext.HASH_SHA256)
    hasher.update(pairing_client_secret)
    var client_secret_hash = hasher.finish()
    var client_response = server_challenge + client_secret_hash + server_signature
    var encrypted_resp = _encrypt_ecb(client_response, pairing_aes_key)
    var enc_b64 = Marshalls.raw_to_base64(encrypted_resp)
    var url = "http://%s:%d/pair?devicename=roth&updateState=1&serverchallengeresp=%s" % [server_ip, DEFAULT_SERVER_PORT, enc_b64]

    var resp = await _http_get(url)
    if not resp.success:
        return false

    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return false

    var root = xml.get_node()
    if root.get_child(0).get_text() != "1":
        return false
    pairing_pairing_secret = Marshalls.base64_to_raw(root.get_child(1).get_text())

    var server_secret = pairing_pairing_secret.slice(0, 16)
    var server_sig = pairing_pairing_secret.slice(16, 48)

    if not _verify_signature(server_secret, server_sig, pairing_server_cert):
        error_message = "MITM detected"
        return false

    return true

func _verify_signature(data: PackedByteArray, signature: PackedByteArray, cert_pem: String) -> bool:
    var cert = X509Certificate.new()
    if cert.load_from_string(cert_pem) != OK:
        return false
    var hasher = HashingContext.new()
    hasher.start(HashingContext.HASH_SHA256)
    hasher.update(data)
    var hash = hasher.finish()
    var crypto = Crypto.new()
    return crypto.verify(HashingContext.HASH_SHA256, hash, signature, cert)

func _sign_message(message: PackedByteArray) -> PackedByteArray:
    var key = CryptoKey.new()
    key.load_from_string(client_key)
    var hasher = HashingContext.new()
    hasher.start(HashingContext.HASH_SHA256)
    hasher.update(message)
    var hash = hasher.finish()
    var crypto = Crypto.new()
    return crypto.sign(HashingContext.HASH_SHA256, hash, key)

func _pairing_stage_4() -> bool:
    pairing_progress = 4
    var sig = _sign_message(pairing_client_secret)
    pairing_client_pairing_secret = pairing_client_secret + sig
    var b64 = Marshalls.raw_to_base64(pairing_client_pairing_secret)
    var url = "http://%s:%d/pair?devicename=roth&updateState=1&clientpairingsecret=%s" % [server_ip, DEFAULT_SERVER_PORT, b64]

    var resp = await _http_get(url)
    if not resp.success:
        return false

    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return false

    var root = xml.get_node()
    return root.get_child(0).get_text() == "1"

func _pairing_stage_5() -> bool:
    pairing_progress = 5
    var url = "https://%s:%d/pair?devicename=roth&updateState=1&phrase=pairchallenge" % [server_ip, DEFAULT_SERVER_PORT]
    var http = HTTPRequest.new()
    add_child(http)
    http.use_ssl = true
    http.ssl_verify_enabled = false  # 忽略自签名证书错误（生产环境应验证）
    var err = http.request(url)
    if err != OK:
        http.queue_free()
        return false
    await http.request_completed
    var success = http.get_response_code() == 200
    http.queue_free()
    if not success:
        return false

    var xml = XMLParser.new()
    if xml.parse(http.get_response_body_as_string()) != OK:
        return false

    var root = xml.get_node()
    return root.get_child(0).get_text() == "1"

# ==================== 其他功能（完全保留） ====================

func get_server_info() -> ServerInfo:
    var url = "http://%s:%d/serverinfo?uniqueid=0123456789ABCDEF&uuid=%s" % [server_ip, DEFAULT_SERVER_PORT, _generate_uuid()]
    var resp = await _http_get(url)
    if not resp.success:
        return server_info
    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return server_info
    var root = xml.get_node()
    if root.get_child_count() < 6:
        return server_info
    server_info.hostname = root.get_child(0).get_text()
    server_info.https_port = int(root.get_child(1).get_text())
    server_info.state = _parse_server_state(root.get_child(2).get_text())
    server_info.current_game = int(root.get_child(3).get_text())
    server_info.gfe_version = root.get_child(4).get_text()
    server_info.pair_status = int(root.get_child(5).get_text())
    return server_info

func _parse_server_state(state: String) -> ServerState:
    match state:
        "SERVER_READY": return ServerState.SERVER_READY
        "SERVER_BUSY": return ServerState.SERVER_BUSY
        _: return ServerState.SERVER_OFFLINE

func _generate_uuid() -> String:
    return Marshalls.raw_to_base64(Crypto.new().generate_random_bytes(16))

func get_app_list() -> Array:
    var url = "http://%s:%d/applist?uniqueid=0123456789ABCDEF&uuid=%s" % [server_ip, DEFAULT_SERVER_PORT, _generate_uuid()]
    var resp = await _http_get(url)
    if not resp.success:
        return app_list
    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return app_list
    var root = xml.get_node()
    app_list = []
    for i in range(root.get_child_count()):
        var node = root.get_child(i)
        if node.get_name() == "App":
            var app = AppInfo.new()
            app.title = node.get_child(0).get_text()
            app.id = int(node.get_child(1).get_text())
            app.is_hdr_supported = node.get_child(2).get_text() == "1"
            app.is_app_collector_game = node.get_child(3).get_text() == "1"
            app_list.append(app)
    return app_list

func launch_app(app_id: int, parent_path: String = "") -> bool:
    if connection_state == ConnectionState.CONNECTED:
        error_message = "Already connected"
        return false
    connection_state = ConnectionState.CONNECTING
    var server_info = await get_server_info()
    if server_info.state == ServerState.SERVER_BUSY:
        if not await resume_connection():
            error_message = "Failed to resume connection"
            connection_state = ConnectionState.DISCONNECTED
            return false

    var rikey = Marshalls.raw_to_base64(pairing_aes_key)
    var rikeyid = Marshalls.raw_to_base64(pairing_salt.slice(0, 4))
    var url = "http://%s:%d/launch?appid=%d&mode=1920x1080x60&rikey=%s&rikeyid=%s&localAudioPlayMode=1" % [server_ip, DEFAULT_SERVER_PORT, app_id, rikey, rikeyid]
    var resp = await _http_get(url)
    if not resp.success:
        connection_state = ConnectionState.DISCONNECTED
        return false

    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        connection_state = ConnectionState.DISCONNECTED
        return false
    var root = xml.get_node()
    current_session.session_url = root.get_child(0).get_text()
    current_session.udp_port = int(root.get_child(1).get_text())
    current_session.tcp_port = int(root.get_child(2).get_text())

    moonlight_stream = MoonlightStream.new()
    moonlight_stream.set_host_address(server_ip)
    moonlight_stream.set_app_id(app_id)
    moonlight_stream.set_resolution(1920, 1080)
    moonlight_stream.set_fps(60)
    moonlight_stream.set_bitrate_kbps(20000)
    moonlight_stream.set_video_codec(MoonlightStream.CODEC_H264)
    moonlight_stream.set_color_space(MoonlightStream.COLOR_SPACE_REC_709)
    moonlight_stream.set_enable_hdr(false)

    var aes_key = pairing_aes_key
    var aes_iv = pairing_salt.slice(0, 4)
    moonlight_stream.set_remote_input_aes_key(aes_key)
    moonlight_stream.set_remote_input_aes_iv(aes_iv)

    if parent_path == "":
        add_child(moonlight_stream)
    else:
        var parent_node = get_node_or_null(parent_path)
        if parent_node:
            parent_node.add_child(moonlight_stream)
        else:
            error_message = "Parent node not found: " + parent_path
            return false

    if not moonlight_stream.start_connection():
        error_message = "Failed to start connection"
        connection_state = ConnectionState.DISCONNECTED
        return false

    connection_state = ConnectionState.CONNECTED
    current_app_id = app_id
    return true

func resume_connection() -> bool:
    var url = "http://%s:%d/resume?uniqueid=0123456789ABCDEF&uuid=%s" % [server_ip, DEFAULT_SERVER_PORT, _generate_uuid()]
    var resp = await _http_get(url)
    if not resp.success:
        return false
    var xml = XMLParser.new()
    if xml.parse(resp.body) != OK:
        return false
    var root = xml.get_node()
    return root.get_child(0).get_text() == "200"

func stop_streaming() -> bool:
    if connection_state != ConnectionState.CONNECTED:
        error_message = "Not connected"
        return false
    connection_state = ConnectionState.DISCONNECTING
    if moonlight_stream:
        moonlight_stream.stop_connection()
        moonlight_stream.queue_free()
        moonlight_stream = null
    var url = "http://%s:%d/cancel?uniqueid=0123456789ABCDEF&uuid=%s" % [server_ip, DEFAULT_SERVER_PORT, _generate_uuid()]
    await _http_get(url)
    connection_state = ConnectionState.DISCONNECTED
    return true

func get_app_asset(app_id: int) -> Texture:
    var url = "http://%s:%d/appasset?appid=%d&AssetType=2&AssetIdx=0" % [server_ip, DEFAULT_SERVER_PORT, app_id]
    var http = HTTPRequest.new()
    add_child(http)
    var err = http.request(url)
    if err != OK:
        http.queue_free()
        return null
    await http.request_completed
    if http.get_response_code() != 200:
        http.queue_free()
        return null
    var img_data = http.get_response_body()
    http.queue_free()
    var image = Image.new()
    if image.load_jpg_from_buffer(img_data) != OK and image.load_png_from_buffer(img_data) != OK:
        return null
    return ImageTexture.create_from_image(image)

# ==================== Getter Functions ====================

func get_error_message() -> String:
    return error_message

func is_pairing_successful() -> bool:
    return pairing_state == PairingState.SUCCESS

func get_pairing_progress() -> int:
    return pairing_progress * 20

func get_current_connection_state() -> ConnectionState:
    return connection_state

func get_current_app_id() -> int:
    return current_app_id

func get_current_session() -> SessionInfo:
    return current_session

func get_current_app_asset() -> Texture:
    return current_app_asset

func set_IP(ip: String):
    server_ip = ip
