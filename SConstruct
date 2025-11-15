#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

def print_error(msg):
    print(f"\033[91mERROR: {msg}\033[0m", file=sys.stderr)

# === 插件配置 ===
libname = "Moonlight-Godot"
projectdir = "demo"  # 可选：用于自动复制到 demo 项目
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

# # === Web 平台直接跳过构建 ===
# if platform == "web":
#     print("[Info] Web platform skipped.")
#     moonlight_built = False
# else:
moonlight_built = True

# 构建目录（分离 static/shared）
static_build_dir = f"{build_root}/moonlight-static-{platform}-{arch}-{build_type}"
shared_build_dir = f"{build_root}/moonlight-shared-{platform}-{arch}-{build_type}"

Path(static_build_dir).mkdir(parents=True, exist_ok=True)
Path(shared_build_dir).mkdir(parents=True, exist_ok=True)

# CMake 基础参数
cmake_base_args = [
    "cmake",
    "-S", str(moonlight_src),
    f"-DCMAKE_BUILD_TYPE={build_type}",
]

if platform == "windows":
    cmake_base_args += ["-A", "x64" if arch == "x86_64" else "Win32"]
    cmake_base_args += ["-DCMAKE_C_FLAGS=/wd5287"]  # 抑制 ENet 警告

# === 构建静态库 ===
print("[CMake] Building STATIC library...")
static_args = cmake_base_args + ["-B", static_build_dir, "-DBUILD_SHARED_LIBS=OFF"]
ret = subprocess.run(static_args, env=os.environ)
if ret.returncode != 0:
    sys.exit(f"Static CMake configure failed ({ret.returncode})")
ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"Static build failed ({ret.returncode})")

# === 构建共享库 ===
print("[CMake] Building SHARED library...")
shared_args = cmake_base_args + ["-B", shared_build_dir, "-DBUILD_SHARED_LIBS=ON"]
ret = subprocess.run(shared_args, env=os.environ)
if ret.returncode != 0:
    sys.exit(f"Shared CMake configure failed ({ret.returncode})")
ret = subprocess.run(["cmake", "--build", shared_build_dir, "--config", build_type], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"Shared build failed ({ret.returncode})")

# === 库文件路径映射 ===
if platform == "windows":
    static_lib_dir = os.path.join(static_build_dir, build_type)
    shared_lib_dir = os.path.join(shared_build_dir, build_type)
    static_lib_name = "moonlight-common-c.lib"
    shared_lib_name = "moonlight-common-c.dll"
elif platform in ("macos", "ios"):
    static_lib_dir = static_build_dir
    shared_lib_dir = shared_build_dir
    static_lib_name = "libmoonlight-common-c.a"
    shared_lib_name = "libmoonlight-common-c.dylib"
else:  # linux
    static_lib_dir = static_build_dir
    shared_lib_dir = shared_build_dir
    static_lib_name = "libmoonlight-common-c.a"
    shared_lib_name = "libmoonlight-common-c.so"

static_lib_full = os.path.join(static_lib_dir, static_lib_name)
shared_lib_full = os.path.join(shared_lib_dir, shared_lib_name)

if not os.path.isfile(static_lib_full):
    sys.exit(f"ERROR: Static library not found: {static_lib_full}")
if not os.path.isfile(shared_lib_full):
    sys.exit(f"ERROR: Shared library not found: {shared_lib_full}")

# === 加载 godot-cpp 构建环境 ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

if platform == "windows":
    # --- 统一 CRT（防止 LNK4098）---
    # 注意：Godot-cpp 默认用 /MD 或 /MDd，所以我们不改 CRT，只排除冲突库
    if is_debug:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:MSVCRT", "/NODEFAULTLIB:LIBCMT"])
    else:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:MSVCRTD", "/NODEFAULTLIB:LIBCMTD"])

    # --- OpenSSL 路径 ---
    openssl_root = os.environ.get("OPENSSL_ROOT_DIR")
    if not openssl_root:
        raise Exception("环境变量 OPENSSL_ROOT_DIR 未设置！")

    crt_subdir = "MDd" if is_debug else "MD"
    openssl_lib_dir = os.path.join(openssl_root, "lib", "VC", "x64", crt_subdir)

    # --- Moonlight 静态库路径 ---
    moonlight_build = "build/moonlight-static-windows-x86_64-Debug" if is_debug else "build/moonlight-static-windows-x86_64-Release"
    enet_lib_dir = os.path.join(moonlight_build, "enet", "Debug" if is_debug else "Release")
    moonlight_lib_dir = os.path.join(moonlight_build, "Debug" if is_debug else "Release")

    # --- 添加库路径和库名 ---
    env.Append(
        LIBPATH=[
            moonlight_lib_dir,
            enet_lib_dir,
            openssl_lib_dir,
        ],
        LIBS=[
            "moonlight-common-c",
            "enet",
            "libcrypto",          # 来自 OpenSSL MDd/MD 目录
            "ws2_32",
            "winmm",
            "crypt32",
            "advapi32",
        ]
    )

# === 添加源码和头文件路径 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))

# === 构建 GDExtension 共享库 ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{platform}/{plugin_filename}"

library = env.SharedLibrary(target=output_path, source=sources)

# === 链接与依赖处理 ===
if moonlight_built:
    # 链接静态库（嵌入符号）
    env.Append(LIBPATH=[static_lib_dir])
    env.Append(LIBS=["moonlight-common-c"])
    Depends(library, static_lib_full)

    # 使用 SCons 复制共享库（确保构建系统追踪依赖）
    plugin_bin_dir = f"bin/{platform}"
    shared_lib_dst = os.path.join(plugin_bin_dir, shared_lib_name)
    copy_shared = env.Command(shared_lib_dst, shared_lib_full, Copy("$TARGET", "$SOURCE"))
    Depends(library, copy_shared)
    Default(copy_shared)

# === 可选：复制到 demo 项目 ===
demo_plugin_dir = f"{projectdir}/bin/{platform}"
demo_copy = env.Install(demo_plugin_dir, library)
Default(demo_copy)

if moonlight_built and platform == "windows":
    demo_dll_copy = env.Install(demo_plugin_dir, shared_lib_dst)
    Default(demo_dll_copy)

# === 最终输出 ===
print(f"[Success] GDExtension built at: {output_path}")