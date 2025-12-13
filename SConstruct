#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path
from SCons.Script import * # 确保添加了这行以使用 SCons 内部函数

def print_error(msg):
    print(f"\033[91mERROR: {msg}\033[0m", file=sys.stderr)

# === 插件配置 ===
libname = "Moonlight-Godot"
projectdir = "demo"
godot_cpp_path = "src/lib/godot-cpp"
moonlight_src = Path("src/lib/moonlight-common-c").resolve()
build_root = "build"

# === FFmpeg 静态链接配置 (新增) ===
# 假设 FFmpeg 库通过 CI 脚本下载到了 lib/ffmpeg/
FFMPEG_ROOT = Path("src/lib/ffmpeg")
FFMPEG_LIBS_BASE = [
    'avcodec', 
    'avformat', 
    'avutil', 
    'swscale', 
    'swresample',
]
# ==================================


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
# ... (此部分保持用户原有的 moonlight-common-c CMake 构建逻辑，未做任何修改)
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

elif platform == "ios":
    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        # "-DUSE_MBEDTLS=ON",  # 强制
        "-B", static_build_dir
    ]
    
    cmake_base_args += ["-DCMAKE_SYSTEM_NAME=iOS"]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Static build failed")
        
        
elif platform == "macos":
    if arch == "universal":
        # 分别构建 x86_64 和 arm64
        build_dirs = []
        for subarch in ["x86_64", "arm64"]:
            subdir = f"{build_root}/moonlight-static-macos-{subarch}-{build_type}"
            Path(subdir).mkdir(parents=True, exist_ok=True)
            cmake_args = [
                "cmake",
                "-S", str(moonlight_src),
                "-B", subdir,
                f"-DCMAKE_BUILD_TYPE={build_type}",
                "-DBUILD_SHARED_LIBS=OFF",
                "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
                # "-DUSE_MBEDTLS=ON",
                f"-DCMAKE_OSX_ARCHITECTURES={subarch}",
            ]
            ret = subprocess.run(cmake_args, env=os.environ)
            if ret.returncode != 0:
                sys.exit(f"CMake configure failed for {subarch}")
            ret = subprocess.run(["cmake", "--build", subdir, "--config", build_type], env=os.environ)
            if ret.returncode != 0:
                sys.exit(f"Build failed for {subarch}")
            build_dirs.append(subdir)

        # 合并成 universal 库
        lib_x86 = os.path.join(build_dirs[0], "libmoonlight-common-c.a")
        lib_arm = os.path.join(build_dirs[1], "libmoonlight-common-c.a")
        final_lib = os.path.join(static_build_dir, "libmoonlight-common-c.a")
        ret = subprocess.run(["lipo", "-create", "-output", final_lib, lib_x86, lib_arm])
        if ret.returncode != 0:
            sys.exit("Failed to create universal library with lipo")
    else:
        # 单架构
        cmake_base_args += [
            "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
            "-DUSE_MBEDTLS=ON",
            f"-DCMAKE_OSX_ARCHITECTURES={arch}",
            "-B", static_build_dir
        ]
        ret = subprocess.run(cmake_base_args, env=os.environ)
        if ret.returncode != 0:
            sys.exit("CMake configure failed")
        ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
        if ret.returncode != 0:
            sys.exit("Static build failed")
            
elif platform == "android":
    # ndk_root = os.environ.get("ANDROID_NDK_ROOT")
    # if not ndk_root:
    #   sys.exit("ERROR: ANDROID_NDK_ROOT must be set for Android builds.")

    # 根据 arch 映射 ABI
    abi_map = {
        "arm64": "arm64-v8a",
        "arm32": "armeabi-v7a",
        "x86": "x86",
        "x86_64": "x86_64"
    }
    android_abi = abi_map.get(arch)
    if not android_abi:
        sys.exit(f"Unsupported Android arch: {arch}")

    cmake_base_args += [
        "-DCMAKE_POSITION_INDEPENDENT_CODE=ON",
        "-DUSE_MBEDTLS=ON",
        "-B", static_build_dir,
        # f"-DCMAKE_TOOLCHAIN_FILE={ndk_root}/build/cmake/android.toolchain.cmake",
        f"-DANDROID_ABI={android_abi}",
        "-DANDROID_PLATFORM=21",  # 最低 API，根据需要调整
        "-DANDROID_STL=c++_shared"
    ]

    ret = subprocess.run(cmake_base_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit("CMake configure failed for Android")
    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit("Android static build failed")
        
else:  # linux
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


# === FFmpeg 链接配置逻辑 (新增) ===

FFMPEG_ARCH_KEY = ""
FFMPEG_EXTRA_LIBS = []

if platform == 'windows':
    FFMPEG_ARCH_KEY = 'win64'
    # FFMPEG_EXTRA_LIBS = ['z']
elif platform == 'linux':
    if arch == 'x86_64':
        FFMPEG_ARCH_KEY = 'linux64'
    elif arch == 'arm64':
        FFMPEG_ARCH_KEY = 'linuxarm64'
    # FFMPEG_EXTRA_LIBS = ['z', 'm', 'pthread'] # 将原本的 'm', 'pthread' 移到这里，在 linux 平台保持一致性
    FFMPEG_EXTRA_LIBS = ['pthread'] # 将原本的 'm', 'pthread' 移到这里，在 linux 平台保持一致性
elif platform == 'macos':
    FFMPEG_ARCH_KEY = 'linuxarm64'
    FFMPEG_EXTRA_LIBS = ['iconv']
    
# --- Android 支持 ---
elif platform == 'android':
    if arch == 'x86_64':
        FFMPEG_ARCH_KEY = 'linux64'
    elif arch == 'arm64':
        FFMPEG_ARCH_KEY = 'linuxarm64'
    # Android 平台依赖：log (Android日志), z (zlib), m (数学库)
    FFMPEG_EXTRA_LIBS = ['z', 'log', 'm'] 

# --- iOS 支持 ---
elif platform == 'ios':
    if arch == 'arm64':
        FFMPEG_ARCH_KEY = 'linuxarm64'
    # iOS 平台依赖：z (zlib), bz2
    FFMPEG_EXTRA_LIBS = ['z', 'bz2']
    # iOS 链接所需的系统 Frameworks
    env.Append(FRAMEWORKS=["CoreMedia", "VideoToolbox", "CoreFoundation", "Security"])


FFMPEG_LIB_DIR = FFMPEG_ROOT / FFMPEG_ARCH_KEY
FFMPEG_INC_FULL_PATH = FFMPEG_LIB_DIR / 'include'
FFMPEG_LIB_FULL_PATH = FFMPEG_LIB_DIR / 'lib'

if FFMPEG_LIB_FULL_PATH.is_dir():
    print(f"[Info] FFmpeg static library found at: {FFMPEG_LIB_DIR}")
    
    # 1. 添加 FFmpeg 头文件路径
    env.Append(CPPPATH=[FFMPEG_INC_FULL_PATH.as_posix()])
    
    # 2. 添加 FFmpeg 库文件路径
    env.Append(LIBPATH=[FFMPEG_LIB_FULL_PATH.as_posix()])

    # 3. 链接 FFmpeg 核心库及其依赖
    FFMPEG_ALL_LIBS = list(FFMPEG_LIBS_BASE)
    FFMPEG_ALL_LIBS.extend(FFMPEG_EXTRA_LIBS)
    
    # 将 FFmpeg 库添加到 env 中，等待后续 Append(LIBS) 统一链接
    env["FFMPEG_LIBS"] = FFMPEG_ALL_LIBS
    
    # 4. 强制静态链接 (针对 Linux 确保 GCC 运行时库静态链接)
    if platform == 'linux':
        env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++'])
else:
    # 仅在非 CI 环境发出警告，避免 CI 失败
    if os.environ.get('CI') != 'true':
        print_error(f"Warning: FFmpeg headers or libs not found in: {FFMPEG_LIB_DIR}")

# ==================================


# === 加载 godot-cpp 构建环境 (步骤 ①) ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 平台特定链接配置 (步骤 ③ - 链接) ===
FFMPEG_LIBS = env.get("FFMPEG_LIBS", []) # 获取 FFmpeg 库列表

if platform == "windows":
    if is_debug:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMTD", "/NODEFAULTLIB:LIBCMT"])
    else:
        env.Append(LINKFLAGS=["/NODEFAULTLIB:LIBCMT", "/NODEFAULTLIB:LIBCMTD"])

    enet_lib_dir = os.path.join(static_build_dir, "enet", build_type)
    # 原有 libs + FFMPEG_LIBS
    libs = ["moonlight-common-c", "enet", "ws2_32", "winmm", "crypt32", "advapi32"]
    libs.extend(FFMPEG_LIBS)
    
    if not use_mbedtls:
        # 链接 OpenSSL
        crt_subdir = "MDd" if is_debug else "MD"
        # ⚠️ 注意: 这里的 OPENSSL_ROOT_DIR 必须在 env 中
        openssl_root = os.environ.get("OPENSSL_ROOT_DIR", "C:\\Program Files\\OpenSSL-Win64") 
        openssl_lib_dir = os.path.join(openssl_root, "lib", "VC", "x64" if arch == "x86_64" else "arm64", crt_subdir)
        env.Append(LIBPATH=[openssl_lib_dir])
        libs.extend(["libcrypto", "libssl"])  # 注意：Windows 下是 libcrypto.lib / libssl.lib

    env.Append(
        LIBPATH=[static_lib_dir, enet_lib_dir],
        LIBS=libs
    )

elif platform == "linux":
    # 将 m 和 pthread 移至 FFMPEG_EXTRA_LIBS 中，以确保 FFmpeg 依赖被正确链接。
    # 这里的 LIBS 只包含 moonlight-common-c
    libs = ["moonlight-common-c"]
    libs.extend(FFMPEG_LIBS)
    env.Append(LIBPATH=[static_lib_dir], LIBS=libs)

elif platform == "android":
    # 原有 LIBS: ["moonlight-common-c"]
    libs = ["moonlight-common-c"]
    libs.extend(FFMPEG_LIBS) # 包含 z, log, m, avcodec, ...
    env.Append(LIBPATH=[static_lib_dir], LIBS=libs)

elif platform == "macos":
    # 原有 LIBS: ["moonlight-common-c"], FRAMEWORKS=["Security", "CoreFoundation"]
    libs = ["moonlight-common-c"]
    libs.extend(FFMPEG_LIBS) # 包含 iconv, avcodec, ...
    env.Append(LIBPATH=[static_lib_dir], LIBS=libs, FRAMEWORKS=["Security", "CoreFoundation"])

elif platform == "ios":
    # 原有 LIBS: ["moonlight-common-c"]
    libs = ["moonlight-common-c"]
    libs.extend(FFMPEG_LIBS) # 包含 z, bz2, avcodec, ...
    # FRAMEWORKS 已在上方 FFmpeg 配置段中添加
    env.Append(LIBPATH=[static_lib_dir], LIBS=libs)

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

# === 构建 GDExtension (步骤 ③ - 构建) ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{platform}/{plugin_filename}"

library = env.SharedLibrary(target=output_path, source=sources)
Depends(library, static_lib_full)
# === FFmpeg 动态库 (DLL/SO) 拷贝 (新增) ===

def copy_ffmpeg_dlls(to_bin = False):
    # 1. 确定平台键
    # 假设 arch 变量已被正确设置为 x86_64, arm64 等
    platform_key = platform
    if platform == "windows":
        # 转换为 win64, linux64, linuxarm64 等
        platform_key = "win" + ("64" if arch == "x86_64" else "32")
    elif platform == "linux":
        # 转换为 linux64, linuxarm64 等
        platform_key = "linux" + ("64" if arch == "x86_64" else "arm64")
    elif platform == "macos":
        # macOS 统一使用 macos64 (您可以根据实际情况调整)
        platform_key = "linuxarm64" 
        
    # 2. 定义源目录和目标目录
    FFMPEG_ROOT = Path("src/lib/ffmpeg")
    # FFmpeg DLLs 的源目录 (例如 src/lib/ffmpeg/win64/bin)
    FFMPEG_DLL_SRC_DIR = FFMPEG_ROOT / platform_key / "bin"
    # 目标安装路径： Godot GDExtension 的部署目录，通常是 bin/
    FFMPEG_DLL_DST_DIR = Path("bin") 

    # 3. 确定要拷贝的文件模式
    if platform == "windows":
        # 拷贝所有主要的 FFmpeg DLLs (*.dll)
        dll_patterns = [
            "avcodec-*.dll", "avdevice-*.dll", "avfilter-*.dll", 
            "avformat-*.dll", "avutil-*.dll", "swresample-*.dll", 
            "swscale-*.dll"
        ]
        dll_sources = []
        print(f"!!!!!FFmpeg DLLs to be copied: {dll_patterns}")
        for pattern in dll_patterns:
            # 使用 SCons 的 Glob 函数查找文件
            dll_sources.extend(Glob(str(FFMPEG_DLL_SRC_DIR / pattern)))
            
    elif platform == "linux":
        # 拷贝所有 lib*.so* 文件
        dll_sources = Glob(str(FFMPEG_DLL_SRC_DIR / "lib*.so*"))
        
    elif platform == "macos":
        # 拷贝所有 lib*.dylib 文件
        dll_sources = Glob(str(FFMPEG_DLL_SRC_DIR / "lib*.dylib"))
        
    else:
        dll_sources = []
    print(f"FFmpeg DLLs to be copied: {dll_sources}")
    # 4. 执行拷贝
    if dll_sources:
        # # 确保目标部署路径存在
        # env.Execute(Mkdir(str(FFMPEG_DLL_DST_DIR))) 
        FFMPEG_DLL_DST_DIR = f"{projectdir}/addons/{libname}/bin/{platform}" if not to_bin else f"/bin"
        print(f"安装 FFmpeg 动态库到 {FFMPEG_DLL_DST_DIR}")
        
        # env.Install() 负责将文件拷贝到指定目录，并确保其依赖于构建目标
        install_targets = env.Install(
            str(FFMPEG_DLL_DST_DIR),
            dll_sources
        )
        Default(install_targets)

copy_ffmpeg_dlls()
copy_ffmpeg_dlls(to_bin=True)

# === 复制到 demo (步骤 ④) ===
demo_plugin_dir = f"{projectdir}/addons/{libname}/bin/{platform}"
demo_copy = env.Install(demo_plugin_dir, library)
Default(demo_copy)

print(f"[Success] GDExtension built at: {output_path}")