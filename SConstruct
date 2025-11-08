#!/usr/bin/env python
import os
import sys
from methods import print_error

# ==============================
# 配置路径
# ==============================
libname = "Moonlight-Godot"
projectdir = "demo"

godot_cpp_path = "src/lib/godot-cpp"
moonlight_common_c_path = "src/lib/moonlight-common-c"

# ==============================
# 环境初始化
# ==============================
localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)
Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

# 检查子模块
if (not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path))) or \
   (not (os.path.isdir(moonlight_common_c_path) and os.listdir(moonlight_common_c_path))):
    print_error("""godot-cpp and moonlight-common-c is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download them:
    git submodule update --init --recursive""")
    sys.exit(1)

# ==============================
# 初始化 godot-cpp 环境
# ==============================
env = SConscript(godot_cpp_path + "/SConstruct", {"env": env, "customs": customs})

# ==============================
# 构建 moonlight-common-c 静态库
# ==============================

# 源文件收集（等效于 aux_source_directory）
moonlight_src_dir = os.path.join(moonlight_common_c_path, "src")
reedsolomon_dir = os.path.join(moonlight_common_c_path, "reedsolomon")
enet_dir = os.path.join(moonlight_common_c_path, "enet")

moonlight_sources = Glob(os.path.join(moonlight_src_dir, "*.c"))
moonlight_sources += Glob(os.path.join(reedsolomon_dir, "*.c"))

# enet 子项目：需单独构建（递归处理）
enet_sources = Glob(os.path.join(enet_dir, "callbacks.c"))
enet_sources += Glob(os.path.join(enet_dir, "compress.c"))
enet_sources += Glob(os.path.join(enet_dir, "host.c"))
enet_sources += Glob(os.path.join(enet_dir, "list.c"))
enet_sources += Glob(os.path.join(enet_dir, "packet.c"))
enet_sources += Glob(os.path.join(enet_dir, "peer.c"))
enet_sources += Glob(os.path.join(enet_dir, "protocol.c"))
enet_sources += Glob(os.path.join(enet_dir, "unix.c")) if env["platform"] not in ["windows"] else []
enet_sources += Glob(os.path.join(enet_dir, "win32.c")) if env["platform"] == "windows" else []

# 合并所有源
all_moonlight_sources = moonlight_sources + enet_sources

# 创建独立环境用于 moonlight-common-c（避免污染 godot-cpp 环境）
ml_env = env.Clone()

# C 标准
ml_env.Append(CCFLAGS=["-std=c11"])

# 宏定义
ml_env.Append(CPPDEFINES=["HAS_SOCKLEN_T"])

# 调试/Release 定义
if env["target"] in ["template_debug", "editor"]:
    ml_env.Append(CPPDEFINES=["LC_DEBUG"])
else:
    ml_env.Append(CPPDEFINES=["NDEBUG"])
    if ml_env["CC"] and "gcc" in ml_env["CC"]:
        ml_env.Append(CCFLAGS=["-Wno-maybe-uninitialized"])

# # 平台特定编译选项
# if env["platform"] == "windows":
#     if env.get("is_mingw", False):
#         ml_env.Append(LIBS=["ws2_32", "winmm"])
#     else:  # MSVC
#         ml_env.Append(CCFLAGS=["/W3", "/wd4100", "/wd4232", "/wd5105", "/WX"])
#         ml_env.Append(LIBS=["ws2_32", "winmm"])
# else:
#     ml_env.Append(CCFLAGS=["-Wall", "-Wextra", "-Wno-unused-parameter", "-Werror"])
#     # 可选：启用 -fanalyzer（需 GCC >= 10）
#     # if "gcc" in ml_env["CC"] and env.get("code_analysis", False):
#     #     ml_env.Append(CCFLAGS=["-fanalyzer"])

# ==============================
# 平台特定编译选项（修复 MSVC 警告 + 移除无效 -std=c11）
# ==============================

# 移除 -std=c11（MSVC 不支持）
if env["platform"] == "windows" and env.get("CC", "").endswith("cl.exe"):
    # MSVC：不使用 -std=c11
    pass
else:
    ml_env.Append(CCFLAGS=["-std=c11"])

# 宏定义
ml_env.Append(CPPDEFINES=["HAS_SOCKLEN_T"])

# 调试/Release 定义
if env["target"] in ["template_debug", "editor"]:
    ml_env.Append(CPPDEFINES=["LC_DEBUG"])
else:
    ml_env.Append(CPPDEFINES=["NDEBUG"])
    if ml_env["CC"] and "gcc" in ml_env["CC"]:
        ml_env.Append(CCFLAGS=["-Wno-maybe-uninitialized"])

# 平台特定警告与链接设置
if env["platform"] == "windows":
    if env.get("is_mingw", False):
        # MinGW：保留 -std=c11 已处理，正常链接
        ml_env.Append(LIBS=["ws2_32", "winmm"])
    else:
        # MSVC：禁用 enet 相关警告，关闭 /WX 避免中断（或精准禁用）
        ml_env.Append(CCFLAGS=[
            "/W3",
            "/wd4100",   # unreferenced formal parameter
            "/wd4232",   # nonstandard extension
            "/wd5105",   # macro expansion producing 'defined'
            "/wd5287",   # different enum types in bitwise op (enet)
            "/wd4142",   # benign redefinition (QOS_FLOWID)
            "/D_CRT_SECURE_NO_WARNINGS"
            # 注意：移除了 /WX，否则任何警告都会导致失败
        ])
        ml_env.Append(LIBS=["ws2_32", "winmm"])
else:
    ml_env.Append(CCFLAGS=["-Wall", "-Wextra", "-Wno-unused-parameter", "-Werror"])
    # 可选：启用 -fanalyzer（需 GCC >= 10）
    # if "gcc" in ml_env["CC"] and env.get("code_analysis", False):
    #     ml_env.Append(CCFLAGS=["-fanalyzer"])

# 包含路径
ml_env.Append(CPPPATH=[
    moonlight_src_dir,
    reedsolomon_dir,
    os.path.join(enet_dir, "include"),
])

# # 处理 USE_MBEDTLS vs OpenSSL
# use_mbedtls = env.get("use_mbedtls", False)
# if use_mbedtls:
#     ml_env.Append(CPPDEFINES=["USE_MBEDTLS"])
#     # 假设 mbedcrypto 已安装或作为子模块存在
#     # 若为系统库：
#     ml_env.Append(LIBS=["mbedcrypto"])
#     # 若需指定路径，可添加 CPPPATH 和 LIBPATH
# else:
#     # 使用 OpenSSL
#     ml_env.Append(LIBS=["crypto"])
#     # 注意：SCons 不自动查找 OpenSSL，需确保 pkg-config 或手动指定路径
#     # 示例（Linux/macOS）：
#     if env["platform"] not in ["windows"]:
#         try:
#             openssl_inc = env.ParseConfig("pkg-config --cflags openssl")
#             openssl_lib = env.ParseConfig("pkg-config --libs openssl")
#         except Exception:
#             # fallback: assume standard paths
#             ml_env.Append(CPPPATH=["/usr/include", "/usr/local/include"])
#             ml_env.Append(LIBPATH=["/usr/lib", "/usr/local/lib"])

# ==============================
# 处理 USE_MBEDTLS vs OpenSSL（增强版，支持 Windows 路径）
# ==============================
use_mbedtls = env.get("use_mbedtls", False)
if use_mbedtls:
    ml_env.Append(CPPDEFINES=["USE_MBEDTLS"])
    ml_env.Append(LIBS=["mbedcrypto"])
else:
    # OpenSSL 分支
    if env["platform"] == "windows":
        # 支持通过 OPENSSL=... 指定路径
        # print("WARN：使用预先配置的openssl路径，请在GitHub actions中另外配置！！！")
        openssl_root = env.get("openssl", "D:\\CodingENV\\OpenSSL-Win64")
        # openssl_root = 
        if not openssl_root:
            print_error("OpenSSL path not specified for Windows. Please provide OPENSSL=/path/to/OpenSSL-Win64")
            sys.exit(1)

        openssl_inc = os.path.join(openssl_root, "include")
        openssl_lib = os.path.join(openssl_root, "lib")

        if not os.path.exists(os.path.join(openssl_inc, "openssl", "evp.h")):
            print_error(f"openssl/evp.h not found in {openssl_inc}")
            sys.exit(1)

        ml_env.Append(CPPPATH=[openssl_inc])
        ml_env.Append(LIBPATH=[openssl_lib])
        ml_env.Append(LIBS=["libcrypto"])  # 或 "libcrypto_static"，取决于你用的版本
    else:
        # Linux/macOS：尝试 pkg-config
        try:
            env.ParseConfig("pkg-config --cflags --libs libcrypto")
            ml_env.Append(CPPPATH=env["CPPPATH"])
            ml_env.Append(LIBPATH=env["LIBPATH"])
            ml_env.Append(LIBS=["crypto"])
        except Exception:
            # fallback
            ml_env.Append(CPPPATH=["/usr/include", "/usr/local/include"])
            ml_env.Append(LIBPATH=["/usr/lib", "/usr/local/lib"])
            ml_env.Append(LIBS=["crypto"])

# 构建静态库
moonlight_static_lib = ml_env.StaticLibrary(
    target=os.path.join("bin", env["platform"], "libmoonlight-common-c"),
    source=all_moonlight_sources
)

# ==============================
# 构建 GDExtension 主库
# ==============================

env.Append(CPPPATH=["src/"])

# 收集插件 C++ 源码
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))
sources.extend(Glob("src/*/*/*.cpp"))
sources.extend(Glob("src/*/*/*/*.cpp"))

# 文档数据（可选）
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        pass  # pre-4.3

# 链接 moonlight 静态库
env.Append(LIBS=[moonlight_static_lib])
env.Append(LIBPATH=[os.path.join("bin", env["platform"])])

# 生成 GDExtension 共享库
suffix = env['suffix'].replace(".dev", "").replace(".universal", "")
lib_filename = "{}{}{}{}".format(env.subst('$SHLIBPREFIX'), libname, suffix, env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "bin/{}/{}".format(env['platform'], lib_filename),
    source=sources,
)

# 安装到 demo/bin/
copy = env.Install("{}/bin/{}/".format(projectdir, env["platform"]), library)

Default(library, copy)