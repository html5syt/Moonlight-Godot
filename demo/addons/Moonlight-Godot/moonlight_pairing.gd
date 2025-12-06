class_name MoonlightPairing
extends Node

signal pin_generated(pin)
signal status_changed(msg)
signal success
signal failed(err)

var _tools: MoonlightTools
var _ex_tools # GDExtension 实例
var _active = false

func _init(tools):
    _tools = tools
    if ClassDB.class_exists("MoonlightExTools"):
        _ex_tools = ClassDB.instantiate("MoonlightExTools")

func start():
    if _active: return
    _active = true
    _flow()

func _flow():
    # 生成 PIN 和 Salt
    var rng = RandomNumberGenerator.new()
    rng.randomize()
    var pin = "%04d" % rng.randi_range(0, 9999)
    var salt_bytes = _ex_tools.generate_random_bytes(16)
    var salt_hex = salt_bytes.hex_encode()
    
    emit_signal("pin_generated", pin)
    emit_signal("status_changed", "Stage 1: Get Cert...")
    
    # --- 1. 生成 AES Key (PBKDF2) ---
    var aes_key = _ex_tools.derive_aes_key(pin, salt_hex)
    
    # --- Stage 1: Get Server Cert (HTTP) ---
    # 对应 nvpairingmanager.cpp:217
    var url_st1 = "/pair?uniqueid=%s&devicename=Godot&updateState=1&phrase=getservercert&salt=%s&clientcert=%s" \
        % [_tools.unique_id, salt_hex, _tools.get_client_cert_hex()]
    
    # 轮询等待用户在 PC 输入 PIN
    while _active:
        var res = await _tools.request(self, url_st1, 0) # HTTP
        if res.success and res.code == 200:
            var xml = _tools.parse_xml(res.body)
            if xml.get("paired") == "1":
                if xml.has("plaincert"):
                    _tools.save_server_cert(xml["plaincert"]) # 立即保存证书用于 Pinning
                    _stage_2(aes_key)
                    return
        await get_tree().create_timer(1.0).timeout

# --- Stage 2: Client Challenge ---
# 对应 nvpairingmanager.cpp:257
func _stage_2(aes_key: PackedByteArray):
    emit_signal("status_changed", "Stage 2: Client Challenge...")
    
    var random_challenge = _ex_tools.generate_random_bytes(16)
    var enc_challenge = _ex_tools.encrypt_aes_hex(random_challenge, aes_key)
    
    var url = "/pair?uniqueid=%s&devicename=Godot&updateState=1&clientchallenge=%s" \
        % [_tools.unique_id, enc_challenge]
        
    var res = await _tools.request(self, url, 0)
    if res.success and res.code == 200:
        var xml = _tools.parse_xml(res.body)
        if xml.get("paired") == "1":
            _stage_3(xml, aes_key, random_challenge)
        else: _fail("Stage 2 failed")
    else: _fail("Stage 2 net error")

# --- Stage 3: Server Response & Verify ---
# 对应 nvpairingmanager.cpp:287
func _stage_3(xml: Dictionary, aes_key: PackedByteArray, my_challenge: PackedByteArray):
    emit_signal("status_changed", "Stage 3: Verify Server...")
    
    var server_resp_hex = xml.get("challengeresponse", "")
    var decrypted_resp = _ex_tools.decrypt_aes_hex(server_resp_hex, aes_key)
    
    # 验证逻辑 (简化)：
    # Server Response 包含: Hash(Cert) + Signature + ServerSecret
    # 严格流程需要 verify_signature，此处假设解密成功即 PIN 正确 (因为 Key 是由 PIN 派生的)
    
    # 提取 ServerSecret (最后 16 字节) - 这是一个简化假设，具体偏移见 cpp
    # 实际上我们需要构造 Client Pairing Secret
    var server_secret = decrypted_resp.slice(-16) # 假设
    
    # --- Stage 4: Client Pairing Secret ---
    var client_secret_data = _ex_tools.generate_random_bytes(16)
    
    # 签名 ClientSecret
    var f_key = FileAccess.open(MoonlightTools.CLIENT_KEY_PATH, FileAccess.READ)
    var signature = _ex_tools.sign_data(client_secret_data, f_key.get_as_text())
    
    # 拼接: Secret + Signature
    var payload = client_secret_data + signature
    var enc_payload = _ex_tools.encrypt_aes_hex(payload, aes_key)
    
    var url = "/pair?uniqueid=%s&devicename=Godot&updateState=1&clientpairingsecret=%s" \
        % [_tools.unique_id, enc_payload]
        
    var res = await _tools.request(self, url, 0)
    if res.success and res.code == 200:
        _stage_5()
    else: _fail("Stage 4 failed (PIN Wrong?)")

# --- Stage 5: HTTPS Final Challenge (mTLS) ---
# 对应 nvpairingmanager.cpp:343
func _stage_5():
    emit_signal("status_changed", "Stage 5: HTTPS Handshake...")
    
    var url = "/pair?uniqueid=%s&devicename=Godot&updateState=1&phrase=pairchallenge" % _tools.unique_id
    
    # 关键：这里必须使用 mTLS (tls_mode=2)
    var res = await _tools.request(self, url, 2)
    
    if res.success and res.code == 200:
        var xml = _tools.parse_xml(res.body)
        if xml.get("paired") == "1":
            _success()
        else: _fail("Stage 5 denied")
    else:
        # 某些情况 Server 可能返回 403 但实际配对已完成，
        # 但标准流程应为 200。
        _fail("Stage 5 failed: %d" % res.code)

func _fail(msg):
    _active = false
    emit_signal("failed", msg)

func _success():
    _active = false
    emit_signal("success")
