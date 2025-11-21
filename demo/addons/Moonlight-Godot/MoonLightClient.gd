extends Node
class_name MoonlightClient

## 配置常量
const CLIENT_CERT_PATH = "user://addons/moonlight-godot/client.pem"
const CLIENT_KEY_PATH = "user://addons/moonlight-godot/client.key"
const DEFAULT_SERVER_PORT = 47984
const DEFAULT_APP_VERSION = "3.20.3.70" ## Default version, can be updated

## 配对状态
enum PairingState {NOT_STARTED,IN_PROGRESS,SUCCESS,FAILED,CERT_GENERATED}

## 连接状态
enum ConnectionState {DISCONNECTED,CONNECTING,CONNECTED,DISCONNECTING}

## 服务端状态
enum ServerState {SERVER_READY,SERVER_BUSY,SERVER_OFFLINE}

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

## 用于处理配对过程的内部状态
var pairing_state: PairingState = PairingState.NOT_STARTED
var connection_state: ConnectionState = ConnectionState.DISCONNECTED
var server_info: ServerInfo = ServerInfo.new()

## 用于存储证书和密钥
var client_cert: String = ""
var client_key: String = ""

## 用于存储已获取的应用列表
@export var app_list: Array = []

## 用于存储MoonlightStream实例
var moonlight_stream: MoonlightStream = null

## 用于存储当前会话信息
var current_session: SessionInfo = null

## 用于存储当前应用ID
@export var current_app_id: int = 0

## 用于存储当前应用的封面图
var current_app_asset: Texture = null

## 用于存储错误消息
var error_message: String = ""

## 用于存储配对进度
var pairing_progress: int = 0

## 用于存储配对PIN
var pairing_pin: String = ""

## 用于存储配对的随机盐
var pairing_salt: String = ""

## 用于存储配对的AES密钥
var pairing_aes_key: String = ""

## 用于存储配对的客户端随机密钥
var pairing_client_secret: String = ""

## 用于存储配对的服务器签名
var pairing_server_signature: String = ""

## 用于存储配对的服务器证书
var pairing_server_cert: String = ""

## 用于存储配对的服务器响应
var pairing_server_response: String = ""

## 用于存储配对的客户端响应
var pairing_client_response: String = ""

## 用于存储配对的挑战
var pairing_challenge: String = ""

## 用于存储配对的挑战响应
var pairing_challenge_response: String = ""

## 用于存储配对的配对密钥
var pairing_pairing_secret: String = ""

## 用于存储配对的客户端配对密钥
var pairing_client_pairing_secret: String = ""

## 用于存储配对的最终响应
var pairing_final_response: String = ""

## 用于存储配对的最终状态
var pairing_final_state: bool = false

## 存储服务端IP
@export var server_ip: String = "localhost"

func _init():
    ## 初始化时检查证书
    _load_client_certificate()
    ## 创建目录
    var dir = DirAccess.open("user://addons/moonlight-godot")
    if not dir:
        dir = DirAccess.open("user://")
        dir.make_dir_recursive("user://addons/moonlight-godot")
    ## 设置默认值
    pairing_state = PairingState.NOT_STARTED
    connection_state = ConnectionState.DISCONNECTED

func _load_client_certificate() -> void:
    ## 检查并加载客户端证书，如果不存在则生成新证书
    var cert = X509Certificate.new()
    var key = CryptoKey.new()
    
    ## 尝试加载证书
    var cert_error = cert.load(CLIENT_CERT_PATH)
    if cert_error != OK:
        ## 证书不存在，生成新证书
        _generate_client_certificate()
        return
    
    ## 尝试加载密钥
    var key_error = key.load(CLIENT_KEY_PATH)
    if key_error != OK:
        ## 密钥不存在，生成新证书
        _generate_client_certificate()
        return
    
    ## 保存加载的证书和密钥
    client_cert = cert.save_to_string()
    client_key = key.save_to_string()
    
    pairing_state = PairingState.CERT_GENERATED
    error_message = "Loaded existing client certificate"

func _generate_client_certificate() -> void:
    ## 生成新的客户端证书和密钥
    var crypto = Crypto.new()
    ## 生成RSA密钥
    var key = crypto.generate_rsa(2048)
    ## 生成自签名证书
    var cert = crypto.generate_self_signed_certificate(key, "CN=NVIDIA GameStream Client")
    
    ## 保存证书和密钥
    _save_certificate(cert, key)
    
    ## 更新状态
    pairing_state = PairingState.CERT_GENERATED
    error_message = "Generated new client certificate"

func _save_certificate(cert: X509Certificate, key: CryptoKey) -> void:
    ## 保存证书和密钥到文件
    ## 保存证书
    var cert_error = cert.save(CLIENT_CERT_PATH)
    if cert_error != OK:
        error_message = "Failed to save certificate: " + str(cert_error)
        return
    
    ## 保存密钥
    var key_error = key.save(CLIENT_KEY_PATH)
    if key_error != OK:
        error_message = "Failed to save key: " + str(key_error)
        return
    
    ## 更新状态
    pairing_state = PairingState.CERT_GENERATED
    error_message = "Certificate and key saved successfully"

func pair_with_server(pin: String) -> bool:
    ## 执行配对流程，返回是否成功
    if pairing_state == PairingState.SUCCESS:
        error_message = "Already paired with server"
        return false
    pairing_pin = pin
    pairing_state = PairingState.IN_PROGRESS
    pairing_progress = 0
    
    ## 阶段1: 获取服务端证书
    if not _pairing_stage_1():
        error_message = "Failed to get server certificate"
        pairing_state = PairingState.FAILED
        return false
    
    ## 阶段2: 发送加密挑战
    if not _pairing_stage_2():
        error_message = "Failed to send client challenge"
        pairing_state = PairingState.FAILED
        return false
    
    ## 阶段3: 响应服务端挑战
    if not _pairing_stage_3():
        error_message = "Failed to respond to server challenge"
        pairing_state = PairingState.FAILED
        return false
    
    ## 阶段4: 提交客户端配对密钥
    if not _pairing_stage_4():
        error_message = "Failed to submit client pairing secret"
        pairing_state = PairingState.FAILED
        return false
    
    ## 阶段5: 最终验证(失败代表服务器未完成配对！)
    var pair_times = 0
    while not _pairing_stage_5():
        if pair_times > 600:
            error_message = "Failed to complete pairing,server did not respond"
            pairing_state = PairingState.FAILED
            return false
        error_message = "Failed to complete pairing,waiting for server to complete..."
        # pairing_state = PairingState.FAILED
        # return false
        await get_tree().create_timer(1.0).timeout
    
    pairing_state = PairingState.SUCCESS
    error_message = "Pairing successful"
    return true

func _pairing_stage_1() -> bool:
    ## 阶段1: 获取服务端证书
    pairing_progress = 1
    ## 生成随机盐
    pairing_salt = _generate_random_bytes(16)
    
    ## 准备请求
    var url = "http://"+server_ip+":47984/pair?phrase=getservercert&devicename=roth&updateState=1&salt=" + pairing_salt + "&clientcert=" + _encode_base64(client_cert)
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var paired = root.get_child(0).get_text()
    if paired != "1":
        error_message = "Server not paired"
        return false
    
    pairing_server_cert = root.get_child(1).get_text()
    return true

func _pairing_stage_2() -> bool:
    ## 阶段2: 发送加密挑战
    pairing_progress = 2
    ## 生成AES密钥
    pairing_aes_key = _hash_with_pin(pairing_pin)
    
    ## 生成随机挑战
    pairing_challenge = _generate_random_bytes(16)
    
    ## 加密挑战
    var encrypted_challenge = _encrypt_challenge(pairing_challenge, pairing_aes_key)
    
    ## 准备请求
    var url = "http://"+server_ip+":47984/pair?devicename=roth&updateState=1&clientchallenge=" + encrypted_challenge
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var paired = root.get_child(0).get_text()
    if paired != "1":
        error_message = "Server not paired"
        return false
    
    pairing_challenge_response = root.get_child(1).get_text()
    return true

func _pairing_stage_3() -> bool:
    ## 阶段3: 响应服务端挑战
    pairing_progress = 3
    ## 解密挑战响应
    var decrypted_response = _decrypt_challenge(pairing_challenge_response, pairing_aes_key)
    
    ## 提取服务端挑战和签名
    var server_challenge = decrypted_response.substr(0, 16)
    var server_signature = decrypted_response.substr(16, 32)
    
    ## 生成客户端响应
    pairing_client_secret = _generate_random_bytes(16)
    var client_secret_hash = _hash_with_pin(pairing_client_secret)  ## 修复：使用_hash_with_pin()而不是_hash_with_salt()
    var client_response = server_challenge + client_secret_hash + server_signature
    
    ## 加密客户端响应
    var encrypted_client_response = _encrypt_challenge(client_response, pairing_aes_key)
    
    ## 准备请求
    var url = "http://"+server_ip+":47984/pair?devicename=roth&updateState=1&serverchallengeresp=" + encrypted_client_response
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var paired = root.get_child(0).get_text()
    if paired != "1":
        error_message = "Server not paired"
        return false
    
    pairing_pairing_secret = root.get_child(1).get_text()
    
    ## 解析配对密钥
    var server_secret = pairing_pairing_secret.substr(0, 16)
    server_signature = pairing_pairing_secret.substr(16, 32)
    
    ## 验证签名
    if not _verify_signature(server_secret, server_signature, pairing_server_cert):
        error_message = "MITM detected"
        return false
    
    ## 验证PIN
    var expected_response = _generate_expected_response(server_secret, pairing_server_cert, server_challenge)
    if expected_response != pairing_challenge_response:
        error_message = "Incorrect PIN"
        return false
    
    return true

func _pairing_stage_4() -> bool:
    ## 阶段4: 提交客户端配对密钥
    pairing_progress = 4
    ## 生成客户端配对密钥
    pairing_client_pairing_secret = pairing_client_secret + _sign_message(pairing_client_secret)
    
    ## 准备请求
    var url = "http://"+server_ip+":47984/pair?devicename=roth&updateState=1&clientpairingsecret=" + _encode_base64(pairing_client_pairing_secret)
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var paired = root.get_child(0).get_text()
    if paired != "1":
        error_message = "Server not paired"
        return false
    
    return true

func _pairing_stage_5() -> bool:
    ## 阶段5: 最终验证
    pairing_progress = 5
    ## 准备请求
    var url = "https://"+server_ip+":47984/pair?devicename=roth&updateState=1&phrase=pairchallenge"
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var paired = root.get_child(0).get_text()
    if paired != "1":
        error_message = "Server not paired"
        return false
    
    pairing_final_state = true
    return true

func _hash_with_pin(pin: String) -> String:
    ## 使用SHA-256哈希PIN和盐
    return _hash(pin + pairing_salt)

func _hash(data: String) -> String:
    ## 使用SHA-256哈希数据
    var hash_context = HashingContext.new()
    hash_context.hash(data, HashingContext.HASH_SHA256)
    return hash_context.get_digest_as_string()

func _generate_random_bytes(length: int) -> String:
    ## 生成指定长度的随机字节
    var crypto = Crypto.new()
    return crypto.generate_random_bytes(length).hex_encode()

func _encrypt_challenge(challenge: String, key: String) -> String:
    ## 使用AES-ECB加密挑战
    var crypto = Crypto.new()
    ## 将key转为CryptoKey
    var crypto_key = CryptoKey.new()
    crypto_key.load_from_buffer(key.to_utf8_buffer())
    var encrypted = crypto.encrypt(crypto_key, challenge.to_utf8_buffer())
    return _encode_base64(encrypted)

func _decrypt_challenge(encrypted_challenge: String, key: String) -> String:
    ## 使用AES-ECB解密挑战
    var crypto = Crypto.new()
    ## 将key转为CryptoKey
    var crypto_key = CryptoKey.new()
    crypto_key.load_from_buffer(key.to_utf8_buffer())
    var decrypted = crypto.decrypt(crypto_key, _decode_base64(encrypted_challenge))  ## 修复：使用正确的参数类型
    return decrypted

func _verify_signature(data: String, signature: String, server_cert: String) -> bool:
    ## 验证签名
    var crypto = Crypto.new()
    var cert = crypto.load_certificate(server_cert)
    
    ## 计算数据的哈希
    var hash_context = HashingContext.new()
    hash_context.hash(data, HashingContext.HASH_SHA256)
    var hash = hash_context.get_digest()
    
    ## 解码签名
    var signature_bytes = _decode_base64(signature)
    
    return crypto.verify(HashingContext.HASH_SHA256, hash, signature_bytes, cert)

func _sign_message(message: String) -> String:
    ## 签名消息
    var crypto = Crypto.new()
    
    ## 计算消息哈希
    var hash_context = HashingContext.new()
    hash_context.hash(message, HashingContext.HASH_SHA256)
    var hash = hash_context.get_digest()
    
    ## 加载密钥
    var key = CryptoKey.new()
    key.load_from_string(client_key)
    
    var signature = crypto.sign(HashingContext.HASH_SHA256, hash, key)
    return _encode_base64(signature)

func _generate_expected_response(server_secret: String, server_cert: String, server_challenge: String) -> String:
    ## 生成预期响应
    var data = server_challenge + server_secret
    return _hash(data)

func _encode_base64(data: String) -> String:
    ## 编码为Base64
    return Marshalls.utf8_to_base64(data)

func _decode_base64(data: String) -> PackedByteArray:  ## 修复：返回PackedByteArray
    ## 解码Base64
    return Marshalls.base64_to_raw(data)

func get_server_info() -> ServerInfo:
    ## 获取服务器信息
    var url = "http://"+server_ip+":47984/serverinfo?uniqueid=0123456789ABCDEF&uuid=" + _generate_uuid()
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return server_info
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return server_info
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return server_info
    
    var root = xml.get_node()
    server_info.hostname = root.get_child(0).get_text()
    server_info.https_port = int(root.get_child(1).get_text())
    server_info.state = _parse_server_state(root.get_child(2).get_text())
    server_info.current_game = int(root.get_child(3).get_text())
    server_info.gfe_version = root.get_child(4).get_text()
    server_info.pair_status = int(root.get_child(5).get_text())
    return server_info

func _parse_server_state(state: String) -> ServerState:
    ## 解析服务器状态字符串
    if state == "SERVER_READY":
        return ServerState.SERVER_READY
    elif state == "SERVER_BUSY":
        return ServerState.SERVER_BUSY
    else:
        return ServerState.SERVER_OFFLINE

func _generate_uuid() -> String:
    ## 生成UUID
    return _generate_random_bytes(16)

func get_app_list() -> Array:
    ## 获取应用列表
    var url = "http://"+server_ip+":47984/applist?uniqueid=0123456789ABCDEF&uuid=" + _generate_uuid()
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return app_list
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return app_list
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return app_list
    
    var root = xml.get_node()
    app_list = []
    for i in range(root.get_child_count()):
        var app_node = root.get_child(i)
        if app_node.get_name() == "App":
            var app = AppInfo.new()
            app.title = app_node.get_child(0).get_text()
            app.id = int(app_node.get_child(1).get_text())
            app.is_hdr_supported = app_node.get_child(2).get_text() == "1"
            app.is_app_collector_game = app_node.get_child(3).get_text() == "1"
            app_list.append(app)
    return app_list

func launch_app(app_id: int, parent_path: String = "") -> bool:
    ## 启动应用并创建连接，增加parent_path参数
    if connection_state == ConnectionState.CONNECTED:
        error_message = "Already connected"
        return false
    
    connection_state = ConnectionState.CONNECTING
    
    ## 获取服务器信息
    var server_info = get_server_info()
    
    ## 如果服务器正在忙，尝试恢复
    if server_info.state == ServerState.SERVER_BUSY:
        if not resume_connection():
            error_message = "Failed to resume connection"
            connection_state = ConnectionState.DISCONNECTED
            return false
    
    ## 准备请求
    var url = "http://"+server_ip+":47984/launch?appid=" + str(app_id) + "&mode=1920x1080x60&rikey=" + _encode_base64(pairing_aes_key) + "&rikeyid=" + _encode_base64(pairing_salt.substr(0, 4)) + "&localAudioPlayMode=1"
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        connection_state = ConnectionState.DISCONNECTED
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        connection_state = ConnectionState.DISCONNECTED
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        connection_state = ConnectionState.DISCONNECTED
        return false
    
    var root = xml.get_node()
    current_session.session_url = root.get_child(0).get_text()
    current_session.udp_port = int(root.get_child(1).get_text())
    current_session.tcp_port = int(root.get_child(2).get_text())
    
    ## 创建MoonlightStream实例
    moonlight_stream = MoonlightStream.new()
    moonlight_stream.set_host_address(""+server_ip+"")
    moonlight_stream.set_app_id(app_id)
    moonlight_stream.set_resolution(1920, 1080)
    moonlight_stream.set_fps(60)
    moonlight_stream.set_bitrate_kbps(20000)
    moonlight_stream.set_video_codec(MoonlightStream.CODEC_H264)
    moonlight_stream.set_color_space(MoonlightStream.COLOR_SPACE_REC_709)
    moonlight_stream.set_enable_hdr(false)
    
    ## 设置远程输入密钥
    var aes_key = _decode_base64(pairing_aes_key)
    var aes_iv = _decode_base64(pairing_salt.substr(0, 4))
    moonlight_stream.set_remote_input_aes_key(aes_key)
    moonlight_stream.set_remote_input_aes_iv(aes_iv)
    
    ## 添加到指定父节点
    if parent_path == "":
        add_child(moonlight_stream)
    else:
        var parent_node = get_node(parent_path)
        if parent_node:
            parent_node.add_child(moonlight_stream)
        else:
            error_message = "Parent node not found: " + parent_path
            return false
    
    ## 启动连接
    var success = moonlight_stream.start_connection()
    if not success:
        error_message = "Failed to start connection: " + moonlight_stream.get_error_message()
        connection_state = ConnectionState.DISCONNECTED
        return false
    
    connection_state = ConnectionState.CONNECTED
    current_app_id = app_id
    return true

func resume_connection() -> bool:
    ## 尝试恢复已存在的连接
    var url = "http://"+server_ip+":47984/resume?uniqueid=0123456789ABCDEF&uuid=" + _generate_uuid()
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return false
    
    ## 解析响应
    var response = http_client.get_response_body_as_string()
    var xml = XMLParser.new()
    var parse_error = xml.parse(response)
    if parse_error != OK:
        error_message = "Failed to parse XML: " + str(parse_error)
        return false
    
    var root = xml.get_node()
    var status = root.get_child(0).get_text()
    return status == "200"

func stop_streaming() -> bool:
    ## 停止流媒体并断开连接
    if connection_state != ConnectionState.CONNECTED:
        error_message = "Not connected"
        return false
    
    connection_state = ConnectionState.DISCONNECTING
    
    ## 停止连接
    if moonlight_stream:
        moonlight_stream.stop_connection()
        moonlight_stream.queue_free()
        moonlight_stream = null
    
    ## 发送取消请求
    var url = "http://"+server_ip+":47984/cancel?uniqueid=0123456789ABCDEF&uuid=" + _generate_uuid()
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        connection_state = ConnectionState.CONNECTED
        return false
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        connection_state = ConnectionState.CONNECTED
        return false
    
    connection_state = ConnectionState.DISCONNECTED
    return true

func get_app_asset(app_id: int) -> Texture:
    ## 获取应用的封面图
    var url = "http://"+server_ip+":47984/appasset?appid=" + str(app_id) + "&AssetType=2&AssetIdx=0"
    
    ## 发送请求
    var http_client = HTTPClient.new()
    
    ## 修复：使用connect_to_host()而不是connect()
    var host = url.split("://")[1].split("/")[0]
    var port = 80
    if url.startswith("https://"):
        port = 443
    var err = http_client.connect_to_host(host, port)
    
    if err != OK:
        error_message = "Failed to connect to server: " + str(err)
        return null
    
    err = http_client.get_response()
    if err != OK:
        error_message = "Failed to get response: " + str(err)
        return null
    
    ## 获取图像数据
    var image_data = http_client.get_response_body_as_array()
    
    ## 创建Texture
    var image = Image.new()
    image.load_from_buffer(image_data)
    var texture = Texture.new()
    texture.create_from_image(image)
    return texture

func get_error_message() -> String:
    ## 获取当前错误消息
    return error_message

func is_pairing_successful() -> bool:
    ## 检查配对是否成功
    return pairing_state == PairingState.SUCCESS

func get_pairing_progress() -> int:
    ## 获取配对进度（0-100）
    return pairing_progress * 20

func get_current_connection_state() -> ConnectionState:
    ## 获取当前连接状态
    return connection_state

func get_current_app_id() -> int:
    ## 获取当前连接的应用ID
    return current_app_id

func get_current_session() -> SessionInfo:
    ## 获取当前会话信息
    return current_session

func get_current_app_asset() -> Texture:
    ## 获取当前应用的封面图
    return current_app_asset

func set_IP(ip: String):
    ## 设置IP地址
    server_ip = ip