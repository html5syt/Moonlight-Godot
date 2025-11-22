extends Node


#func _ready() -> void:
    #var example := ExampleClass.new()
    #example.print_type(example)
    #example.print_helloworld()
    #var start = Time.get_ticks_usec()
    #example.print_fibonacci(2**31-1)
    #var end = Time.get_ticks_usec()
    #print("Use Time（μs）:",end-start)
    #
    #var stream = MoonlightStream.new()
#
    ## 1. 配置服务器信息、分辨率等...
    #stream.host_address = "127.0.0.1"
    ## ... other configurations ...
#
    ## 2. 【关键步骤】生成并设置加密密钥
    ## 这里只是一个示例，实际中用户需要自己实现安全的密钥生成逻辑。
    #var aes_key = PackedByteArray()
    #var aes_iv = PackedByteArray()
#
    ## 填充16字节的随机密钥 (用户需自行实现安全的随机源)
    #for i in range(16):
        #aes_key.append(randi() % 256)
#
    ## 填充至少4字节的随机IV
    #for i in range(4):
        #aes_iv.append(randi() % 256)
#
    #print(aes_key)
    #print(aes_iv)
#
    ## 将密钥传递给插件
    #stream.set_remote_input_aes_key(aes_key)
    #stream.set_remote_input_aes_iv(aes_iv)
#
    ## 3. 启动连接
    #if stream.start_connection():
        #print("Connection started successfully!")
    #else:
        #print("Connection failed! Check if keys were set correctly.")

@onready var moonlight: MoonLightClient = MoonLightClient.new()

var ip = "127.0.0.1"
var pin = 123456
var appid = 0

func test_connect() -> void:
    moonlight


func ip_set(new_text: String) -> void:
    ip = new_text

func pin_set(new_text: String) -> void:
    pin = int(new_text)

func appid_set(new_text: String) -> void:
    appid = int(new_text)

func pair() -> void:
    moonlight.server_ip = ip
    $GridContainer/LineEdit3.text = moonlight
