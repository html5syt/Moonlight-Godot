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

# === CMake 基础参数 ===
cmake_base_args = [
    "cmake",
    "-S", str(moonlight_src),
    f"-DCMAKE_BUILD_TYPE={build_type}",
    "-DBUILD_SHARED_LIBS=OFF",
]

# === 平台特定 CMake 配置与构建 ===
if platform == "windows":
    cmake_base_args += ["-A", "x64"]
    rt_flag = "/MDd" if is_debug else "/MD"
    # 尽可能传递 /MDd（即使 moonlight-common-c 只读，也尝试）
    cmake_base_args += [
        f"-DCMAKE_C_FLAGS={rt_flag} /wd5287",
        f"-DCMAKE_CXX_FLAGS={rt_flag}",
        "-B", static_build_dir
    ]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"CMake configure failed ({ret.returncode})")

    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Static build failed ({ret.returncode})")

elif platform in ("macos", "ios"):
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        "-B", static_build_dir
    ]
    if platform == "macos":
        cmake_base_args += [f"-DCMAKE_OSX_ARCHITECTURES={arch}"]
    else:  # ios
        cmake_base_args += ["-DCMAKE_SYSTEM_NAME=iOS"]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Static build failed")

else:  # linux, android, etc.
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
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
else:  # linux, android
    static_lib_dir = static_build_dir
    static_lib_name = "libmoonlight-common-c.a"

static_lib_full = os.path.join(static_lib_dir, static_lib_name)
if not os.path.isfile(static_lib_full):
    sys.exit(f"ERROR: Static library not found: {static_lib_full}")

# === 加载 godot-cpp 构建环境 ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 平台特定链接配置 ===
if platform == "windows":
    # ✅ 正确处理 CRT 冲突（关键修复！）
    if is_debug:
        # Debug: 使用 MSVCRTD，禁止静态 CRT 库
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMTD", "/NODEFAULTLIB:LIBCMT"])
    else:
        # Release: 使用 MSVCRT，禁止静态 CRT 库
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMT", "/NODEFAULTLIB:LIBCMTD"])

    openssl_root = os.environ.get("OPENSSL_ROOT_DIR")
    if not openssl_root:
        raise Exception("Environment variable OPENSSL_ROOT_DIR is not set!")

    crt_subdir = "MDd" if is_debug else "MD"
    openssl_lib_dir = os.path.join(openssl_root, "lib", "VC", "x64", crt_subdir)
    enet_lib_dir = os.path.join(static_build_dir, "enet", build_type)

    env.Append(
        LIBPATH=[static_lib_dir, enet_lib_dir, openssl_lib_dir],
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
    env.Append(
        LIBPATH=[static_lib_dir],
        LIBS=["moonlight-common-c", "ssl", "crypto", "m", "pthread"]
    )

elif platform == "android":
    env.Append(
        LIBPATH=[static_lib_dir],
        LIBS=["moonlight-common-c", "ssl", "crypto"]
    )

elif platform == "macos":
    env.Append(
        LIBPATH=[static_lib_dir],
        LIBS=["moonlight-common-c"],
        FRAMEWORKS=["Security", "CoreFoundation"]
    )

elif platform == "ios":
    env.Append(
        LIBPATH=[static_lib_dir],
        LIBS=["moonlight-common-c"]
    )

# === 源码与头文件 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))

# === ✅ 新增：GDExtension 文档支持（Godot 4.3+）===
# if env["target"] in ["editor", "template_debug"]:
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

# === 依赖静态库 ===
Depends(library, static_lib_full)

# === 可选：复制到 demo 项目 ===
demo_plugin_dir = f"{projectdir}/bin/{platform}"
demo_copy = env.Install(demo_plugin_dir, library)
Default(demo_copy)

print(f"[Success] GDExtension built at: {output_path}")