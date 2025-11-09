#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
from pathlib import Path

def print_error(msg):
    print(f"\033[91mERROR: {msg}\033[0m", file=sys.stderr)

# === 插件配置 ===
libname = "Moonlight-Godot"
godot_cpp_path = "src/lib/godot-cpp"

# === 检查 godot-cpp ===
if not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path)):
    print_error("godot-cpp submodule not initialized. Run:\n  git submodule update --init --recursive")
    sys.exit(1)

# === 初始化环境 ===
localEnv = Environment(tools=["default"], PLATFORM="")
customs = ["custom.py"]
customs = [os.path.abspath(p) for p in customs if os.path.exists(p)]
opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)
Help(opts.GenerateHelpText(localEnv))
env = localEnv.Clone()

# === 参数解析 ===
platform = ARGUMENTS.get("platform", "linux")
arch = ARGUMENTS.get("arch", "x86_64")
target = ARGUMENTS.get("target", "template_debug")
is_debug = target in ("editor", "template_debug")
build_type = "Debug" if is_debug else "Release"

print(f"[Info] Building for platform={platform}, arch={arch}, target={target}")

# === Web 平台跳过 ===
if platform == "web":
    print("[Info] Web platform skipped.")
    moonlight_built = False
else:
    moonlight_built = True
    moonlight_src = Path("src/lib/moonlight-common-c").resolve()
    build_root = "build"

    # === 构建目录：分离 static 和 shared ===
    static_build_dir = f"{build_root}/moonlight-static-{platform}-{arch}-{build_type}"
    shared_build_dir = f"{build_root}/moonlight-shared-{platform}-{arch}-{build_type}"

    Path(static_build_dir).mkdir(parents=True, exist_ok=True)
    Path(shared_build_dir).mkdir(parents=True, exist_ok=True)

    cmake_base_args = [
        "cmake",
        "-S", str(moonlight_src),
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]

    if platform == "windows":
        cmake_base_args += ["-A", "x64" if arch == "x86_64" else "Win32"]
        cmake_base_args += ["-DCMAKE_C_FLAGS=/wd5287"]

    # === 1. 构建静态库 ===
    print(f"[CMake] Building STATIC library...")
    static_args = cmake_base_args + [
        "-B", static_build_dir,
        "-DBUILD_SHARED_LIBS=OFF",
    ]
    ret = subprocess.run(static_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Static CMake configure failed ({ret.returncode})")

    ret = subprocess.run(["cmake", "--build", static_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Static build failed ({ret.returncode})")

    # === 2. 构建共享库 ===
    print(f"[CMake] Building SHARED library...")
    shared_args = cmake_base_args + [
        "-B", shared_build_dir,
        "-DBUILD_SHARED_LIBS=ON",
    ]
    ret = subprocess.run(shared_args, env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Shared CMake configure failed ({ret.returncode})")

    ret = subprocess.run(["cmake", "--build", shared_build_dir, "--config", build_type], env=os.environ)
    if ret.returncode != 0:
        sys.exit(f"Shared build failed ({ret.returncode})")

    # === 确定文件路径 ===
    if platform == "windows":
        static_lib_dir = os.path.join(static_build_dir, build_type)
        shared_lib_dir = os.path.join(shared_build_dir, build_type)
        static_lib = "moonlight-common-c.lib"
        shared_lib = "moonlight-common-c.dll"
    elif platform == "macos":
        static_lib_dir = static_build_dir
        shared_lib_dir = shared_build_dir
        static_lib = "libmoonlight-common-c.a"
        shared_lib = "libmoonlight-common-c.dylib"
    else:  # linux
        static_lib_dir = static_build_dir
        shared_lib_dir = shared_build_dir
        static_lib = "libmoonlight-common-c.a"
        shared_lib = "libmoonlight-common-c.so"

    static_lib_full = os.path.join(static_lib_dir, static_lib)
    shared_lib_full = os.path.join(shared_lib_dir, shared_lib)

    if not os.path.isfile(static_lib_full):
        sys.exit(f"ERROR: Static library not found: {static_lib_full}")
    if not os.path.isfile(shared_lib_full):
        sys.exit(f"ERROR: Shared library not found: {shared_lib_full}")

# === 加载 godot-cpp ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 添加源码 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))

# === 输出路径 ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{platform}/{plugin_filename}"

library = env.SharedLibrary(
    target=output_path,
    source=sources,
)

# === 链接静态库 + 复制共享库 ===
if moonlight_built:
    # 链接静态库（用于嵌入符号）
    env.Append(LIBPATH=[static_lib_dir])
    if platform == "windows":
        env.Append(LIBS=["moonlight-common-c"])
        Depends(library, static_lib_full)
    else:
        env.Append(LIBS=["moonlight-common-c"])
        Depends(library, static_lib_full)

    # 复制共享库到插件目录（供 Godot 运行时加载）
    plugin_bin_dir = f"bin/{platform}"
    Path(plugin_bin_dir).mkdir(parents=True, exist_ok=True)
    dest_shared = os.path.join(plugin_bin_dir, os.path.basename(shared_lib_full))
    shutil.copy2(shared_lib_full, dest_shared)
    print(f"[Info] Copied {os.path.basename(shared_lib_full)} to {plugin_bin_dir}")

print(f"[Success] GDExtension built at: {output_path}")