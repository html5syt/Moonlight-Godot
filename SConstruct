#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

def print_error(msg):
    print(f"\033[91mERROR: {msg}\033[0m", file=sys.stderr)

# === 插件配置 ===
libname = "Moonlight-Godot"
projectdir = "demo"
godot_cpp_path = "src/lib/godot-cpp"
moonlight_src = Path("src/lib/moonlight-common-c").resolve()
build_root = "build"

# === 检查 godot-cpp submodule ===
if not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path)):
    print_error("godot-cpp submodule not initialized. Run:\n  git submodule update --init --recursive")
    sys.exit(1)

# === 初始化 SCons 环境 ===
localEnv = Environment(tools=["default"], PLATFORM="")
customs = ["custom.py"]
customs = [os.path.abspath(p) for p in customs if os.path.exists(p)]
opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)
Help(opts.GenerateHelpText(localEnv))
env = localEnv.Clone()

# === 解析构建参数 ===
platform = ARGUMENTS.get("platform", "linux")
arch = ARGUMENTS.get("arch", "x86_64")
target = ARGUMENTS.get("target", "template_debug")
is_debug = target in ("editor", "template_debug")
build_type = "Debug" if is_debug else "Release"

print(f"[Info] Building for platform={platform}, arch={arch}, target={target}")

# === 构建目录（仅 static）===
static_build_dir = f"{build_root}/moonlight-static-{platform}-{arch}-{build_type}"
Path(static_build_dir).mkdir(parents=True, exist_ok=True)

# === CMake 静态库构建 ===
cmake_base_args = [
    "cmake",
    "-S", str(moonlight_src),
    f"-DCMAKE_BUILD_TYPE={build_type}",
    "-DBUILD_SHARED_LIBS=OFF",
    "-DCMAKE_POSITION_INDEPENDENT_CODE=ON", 
]

if platform == "windows":
    cmake_base_args += ["-A", "x64"]
    cmake_base_args += ["-DCMAKE_C_FLAGS=/wd5287"]  # 抑制 ENet 警告

print("[CMake] Building STATIC library...")
ret = subprocess.run(cmake_base_args + ["-B", static_build_dir], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"CMake configure failed ({ret.returncode})")

ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"Static build failed ({ret.returncode})")

# === 库文件路径映射 ===
if platform == "windows":
    static_lib_dir = os.path.join(static_build_dir, build_type)
    static_lib_name = "moonlight-common-c.lib"
elif platform in ("macos", "ios"):
    static_lib_dir = static_build_dir
    static_lib_name = "libmoonlight-common-c.a"
else:  # linux
    static_lib_dir = static_build_dir
    static_lib_name = "libmoonlight-common-c.a"

static_lib_full = os.path.join(static_lib_dir, static_lib_name)
if not os.path.isfile(static_lib_full):
    sys.exit(f"ERROR: Static library not found: {static_lib_full}")

# === 加载 godot-cpp 构建环境 ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 平台特定链接配置 ===
if platform == "windows":
    # CRT 兼容性
    if is_debug:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:MSVCRT", "/NODEFAULTLIB:LIBCMT"])
    else:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:MSVCRTD", "/NODEFAULTLIB:LIBCMTD"])

    # OpenSSL 路径（来自环境变量）
    openssl_root = os.environ.get("OPENSSL_ROOT_DIR")
    if not openssl_root:
        raise Exception("环境变量 OPENSSL_ROOT_DIR 未设置！")

    crt_subdir = "MDd" if is_debug else "MD"
    openssl_lib_dir = os.path.join(openssl_root, "lib", "VC", "x64", crt_subdir)

    # ENet 路径（由 CMake 构建生成）
    enet_lib_dir = os.path.join(static_build_dir, "enet", build_type)

    env.Append(
        LIBPATH=[
            static_lib_dir,
            enet_lib_dir,
            openssl_lib_dir,
        ],
        LIBS=[
            "moonlight-common-c",
            "enet",
            "libcrypto",
            "ws2_32",
            "winmm",
            "crypt32",
            "advapi32",
        ]
    )
elif platform == "linux":
    env.Append(LIBS=["moonlight-common-c", "ssl", "crypto", "m", "pthread"])
    env.Append(LIBPATH=[static_lib_dir])
elif platform in ("macos", "ios"):
    env.Append(LIBS=["moonlight-common-c"])
    env.Append(LIBPATH=[static_lib_dir])
    env.Append(FRAMEWORKS=["Security", "CoreFoundation"])

# === 源码与头文件 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))

# === 构建 GDExtension ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{platform}/{plugin_filename}"

library = env.SharedLibrary(target=output_path, source=sources)

# === 依赖静态库 ===
Depends(library, static_lib_full)

# === 可选：复制到 demo 项目 ===
demo_plugin_dir = f"{projectdir}/bin/{platform}"
demo_copy = env.Install(demo_plugin_dir, library)
Default(demo_copy)

print(f"[Success] GDExtension built at: {output_path}")