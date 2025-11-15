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

# === 决定是否使用 mbedTLS ===
if platform == "windows":
    # Windows: 尊重用户选择，但默认使用 OpenSSL（即 use_mbedtls=false）
    use_mbedtls = ARGUMENTS.get("use_mbedtls", "false").lower() in ("1", "true")
else:
    # 其他平台：强制使用 mbedTLS，忽略用户输入
    use_mbedtls = True

print(f"[Info] Building for platform={platform}, arch={arch}, target={target}")
print(f"[Info] use_mbedtls={use_mbedtls}")

# === 构建目录（仅 static）===
static_build_dir = f"{build_root}/moonlight-static-{platform}-{arch}-{build_type}"
Path(static_build_dir).mkdir(parents=True, exist_ok=True)

# === CMake 基础参数 ===
cmake_base_args = [
    "cmake",
    "-S", str(moonlight_src),
    f"-DCMAKE_BUILD_TYPE={build_type}",
    "-DBUILD_SHARED_LIBS=OFF",
]

# === 平台特定 CMake 配置与构建 ===
if platform == "windows":
    if not use_mbedtls:
        # 必须提供 OPENSSL_ROOT_DIR
        openssl_root = os.environ.get("OPENSSL_ROOT_DIR")
        if not openssl_root:
            sys.exit("ERROR: OPENSSL_ROOT_DIR must be set when building Windows with OpenSSL (use_mbedtls=false).")

    cmake_base_args += ["-A", "x64" if arch == "x86_64" else "ARM64"]
    rt_flag = "/MDd" if is_debug else "/MD"
    cmake_base_args += [
        f"-DCMAKE_C_FLAGS={rt_flag}",
        f"-DCMAKE_CXX_FLAGS={rt_flag}",
        f"-DUSE_MBEDTLS={'ON' if use_mbedtls else 'OFF'}",
        "-B", static_build_dir
    ]

    if not use_mbedtls:
        # 传递 OpenSSL 根路径给 CMake
        cmake_base_args += [f"-DOPENSSL_ROOT_DIR={openssl_root}"]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"CMake configure failed ({ret.returncode})")

    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Static build failed ({ret.returncode})")

elif platform in ("ios", "macos"):
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        # "-DUSE_MBEDTLS=ON",  # 强制
        "-B", static_build_dir
    ]
    
    if platform == "macos":
        actual_arch = "arm64"
        cmake_base_args += [f"-DCMAKE_OSX_ARCHITECTURES={actual_arch}"]
    else:  # ios
        cmake_base_args += ["-DCMAKE_SYSTEM_NAME=iOS"]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Static build failed")

elif platform == "android":
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        # "-DUSE_MBEDTLS=ON",  # 强制
        "-B", static_build_dir
    ]
    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Static build failed")

else:  # linux, android
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        "-DUSE_MBEDTLS=ON",  # 强制
        "-B", static_build_dir
    ]
    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Static build failed")

# === 库文件路径映射 ===
if platform == "windows":
    static_lib_dir = os.path.join(static_build_dir, build_type)
    static_lib_name = "moonlight-common-c.lib"
elif platform in ("macos", "ios"):
    static_lib_dir = static_build_dir
    static_lib_name = "libmoonlight-common-c.a"
else:
    static_lib_dir = static_build_dir
    static_lib_name = "libmoonlight-common-c.a"

static_lib_full = os.path.join(static_lib_dir, static_lib_name)
if not os.path.isfile(static_lib_full):
    sys.exit(f"ERROR: Static library not found: {static_lib_full}")

# === 加载 godot-cpp 构建环境 ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 平台特定链接配置 ===
if platform == "windows":
    if is_debug:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMTD", "/NODEFAULTLIB:LIBCMT"])
    else:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMT", "/NODEFAULTLIB:LIBCMTD"])

    enet_lib_dir = os.path.join(static_build_dir, "enet", build_type)
    libs = ["moonlight-common-c", "enet", "ws2_32", "winmm", "crypt32", "advapi32"]
    
    if not use_mbedtls:
        # 链接 OpenSSL
        crt_subdir = "MDd" if is_debug else "MD"
        openssl_lib_dir = os.path.join(os.environ["OPENSSL_ROOT_DIR"], "lib", "VC", "x64" if arch == "x86_64" else "arm64", crt_subdir)
        env.Append(LIBPATH=[openssl_lib_dir])
        libs.extend(["libcrypto", "libssl"])  # 注意：Windows 下是 libcrypto.lib / libssl.lib

    env.Append(
        LIBPATH=[static_lib_dir, enet_lib_dir],
        LIBS=libs
    )

elif platform == "linux":
    env.Append(LIBPATH=[static_lib_dir], LIBS=["moonlight-common-c", "m", "pthread"])

elif platform == "android":
    env.Append(LIBPATH=[static_lib_dir], LIBS=["moonlight-common-c"])

elif platform == "macos":
    env.Append(LIBPATH=[static_lib_dir], LIBS=["moonlight-common-c"], FRAMEWORKS=["Security", "CoreFoundation"])

elif platform == "ios":
    env.Append(LIBPATH=[static_lib_dir], LIBS=["moonlight-common-c"])

# === 源码与头文件 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))

# === GDExtension 文档支持（Godot 4.3+）===
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        print("Not including class reference as we're targeting a pre-4.3 baseline.")

# === 构建 GDExtension ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{platform}/{plugin_filename}"

library = env.SharedLibrary(target=output_path, source=sources)
Depends(library, static_lib_full)

# === 复制到 demo ===
demo_plugin_dir = f"{projectdir}/bin/{platform}"
demo_copy = env.Install(demo_plugin_dir, library)
Default(demo_copy)

print(f"[Success] GDExtension built at: {output_path}")