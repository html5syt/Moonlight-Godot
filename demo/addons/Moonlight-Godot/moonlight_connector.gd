class_name MoonlightConnector
extends Node

# GDExtension 类名
const EXTENSION_CLASS_NAME = "MoonlightStream"

var _streamer_node: Node = null
var _tools: MoonlightTools

func _init(tools: MoonlightTools):
    _tools = tools

## 启动流媒体会话
func start_session(parent_node: Node, app_id: String, options: Dictionary = {}):
    # 检查 GDExtension 是否存在
    if not ClassDB.class_exists(EXTENSION_CLASS_NAME):
        printerr("错误: 未找到类 '%s'。请确保 GDExtension 已加载。" % EXTENSION_CLASS_NAME)
        return

    stop_session() # 清理旧会话

    # 实例化扩展节点
    _streamer_node = ClassDB.instantiate(EXTENSION_CLASS_NAME)
    _streamer_node.name = "MoonlightStreamer"
    parent_node.add_child(_streamer_node)
    
    # 设置连接参数 (调用 GDExtension 方法)
    _call_safe(_streamer_node, "set_server_address", [_tools.server_ip])
    _call_safe(_streamer_node, "set_app_id", [app_id])
    
    # 传递证书路径供底层 C++ 读取
    _call_safe(_streamer_node, "set_client_certs", [MoonlightTools.CLIENT_CERT_PATH, MoonlightTools.CLIENT_KEY_PATH])
    
    # 视频参数
    var w = options.get("width", 1280)
    var h = options.get("height", 720)
    var fps = options.get("fps", 60)
    var bitrate = options.get("bitrate", 5000)
    _call_safe(_streamer_node, "setup_video", [w, h, fps, bitrate])

    # 开始
    _call_safe(_streamer_node, "start", [])
    print("Moonlight 连接器已启动。")

func stop_session():
    if _streamer_node:
        _call_safe(_streamer_node, "stop", [])
        _streamer_node.queue_free()
        _streamer_node = null

func pause_session():
    if _streamer_node: _call_safe(_streamer_node, "pause", [])

func resume_session():
    if _streamer_node: _call_safe(_streamer_node, "resume", [])

# 辅助安全调用
func _call_safe(obj: Object, method: String, args: Array):
    if obj.has_method(method):
        obj.callv(method, args)
    else:
        printerr("警告: GDExtension 对象缺少方法 '%s'" % method)
