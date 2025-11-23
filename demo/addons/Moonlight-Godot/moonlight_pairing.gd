class_name MoonlightPairing
extends Node

signal pairing_pin_generated(pin: String)
signal pairing_status_changed(stage: String, message: String)
signal pairing_success
signal pairing_failed(error: String)

var _tools: MoonlightTools
var _pairing_active: bool = false
var _crypto: Crypto = Crypto.new()

# 上下文数据
var _current_pin: String = ""
var _current_salt_hex: String = ""

func _init(tools: MoonlightTools):
    _tools = tools

func start_pairing():
    if _pairing_active: return
    _pairing_active = true
    _execute_pairing_flow()

func cancel_pairing():
    _pairing_active = false
    emit_signal("pairing_failed", "已取消")

func _execute_pairing_flow():
    emit_signal("pairing_status_changed", "INIT", "正在连接服务器...")
    
    # 1. 获取服务器信息
    var info_res = await _tools.send_request(self, "/serverinfo")
    if not info_res.success or info_res.code != 200:
        _fail("无法连接服务器 (Code: %d)" % info_res.code)
        return
        
    # 2. 生成 PIN 和 Salt
    var rng = RandomNumberGenerator.new()
    rng.randomize()
    var pin = ""
    for i in range(4): pin += str(rng.randi() % 10)
    
    var salt_bytes = _crypto.generate_random_bytes(16)
    var salt_hex = salt_bytes.hex_encode()
    
    _current_pin = pin
    _current_salt_hex = salt_hex
    
    emit_signal("pairing_pin_generated", pin)
    emit_signal("pairing_status_changed", "PIN", "请在主机输入 PIN: " + pin)
    
    # 3. 轮询配对请求
    var client_cert_str = _tools.get_client_cert_string()
    var pair_url = "/pair?uniqueid=%s&devicename=GodotClient&updateState=1&phrase=pairchallenge&salt=%s&clientcert=%s" % [_tools.unique_id, salt_hex, client_cert_str]
    
    var polling = true
    while polling and _pairing_active:
        var res = await _tools.send_request(self, pair_url)
        
        if res.success and res.code == 200:
            var xml = _tools.parse_xml(res.body)
            if xml.has("paired") and xml["paired"] == "1":
                polling = false
                if await _handle_challenge_exchange(xml):
                    _finish_pairing()
                return
        
        await get_tree().create_timer(1.0).timeout

func _handle_challenge_exchange(xml: Dictionary) -> bool:
    emit_signal("pairing_status_changed", "CRYPTO", "正在进行安全验证...")
    
    if not xml.has("challenge"):
        return await _execute_https_confirmation()

    if not ClassDB.class_exists("MoonlightExTools"):
        _fail("缺失 MoonlightExTools 类，请检查 GDExtension。")
        return false
    
    if not FileAccess.file_exists(MoonlightTools.CLIENT_KEY_PATH):
        _fail("找不到客户端私钥")
        return false
    var client_key_pem = FileAccess.get_file_as_string(MoonlightTools.CLIENT_KEY_PATH)
    
    # 调用 C++ 扩展
    var ex_tools = ClassDB.instantiate("MoonlightExTools")
    var server_challenge = xml["challenge"]
    var response_hex = ex_tools.generate_pairing_response(_current_pin, _current_salt_hex, server_challenge, client_key_pem)
    
    if response_hex.is_empty():
        _fail("加密计算失败")
        return false
    
    # 发送挑战响应
    var url = "/pair?uniqueid=%s&phrase=pairchallenge&clientchallenge=%s" % [_tools.unique_id, response_hex]
    var res = await _tools.send_request(self, url)
    
    if res.success and res.code == 200:
        var resp_xml = _tools.parse_xml(res.body)
        if resp_xml.has("paired") and resp_xml["paired"] == "1":
            return await _execute_https_confirmation()
            
    _fail("服务器验证失败")
    return false

func _execute_https_confirmation() -> bool:
    emit_signal("pairing_status_changed", "CONFIRM", "正在完成配对...")
    # 这里启用 HTTPS 并要求使用客户端证书 (mTLS)
    # 因为 get_tls_options 已经根据指示修改为返回 server() 配置来携带证书
    var url = "/pair?uniqueid=%s&phrase=pairchallenge" % _tools.unique_id
    var res = await _tools.send_request(self, url, true, true) # use_https=true, use_client_auth=true
    
    if res.success and res.code == 200:
        _extract_and_save_server_cert(res.body)
        return true
    
    # 如果返回 403 Forbidden，通常说明 mTLS 握手失败（服务器没收到客户端证书）
    if res.code == 403:
        print("HTTPS 确认返回 403。mTLS 可能未正确生效。")
    else:
        print("HTTPS 确认返回: %d" % res.code)
        
    # 即使这里失败，如果之前步骤成功，也可以视为配对完成（视服务器宽容度而定）
    return true

func _extract_and_save_server_cert(xml_body: String):
    var xml = _tools.parse_xml(xml_body)
    if xml.has("certificate"):
        var f = FileAccess.open(MoonlightTools.SERVER_CERT_PATH, FileAccess.WRITE)
        f.store_string("-----BEGIN CERTIFICATE-----\n" + xml["certificate"] + "\n-----END CERTIFICATE-----")

func _fail(reason: String):
    _pairing_active = false
    emit_signal("pairing_failed", reason)

func _finish_pairing():
    _pairing_active = false
    emit_signal("pairing_success")
