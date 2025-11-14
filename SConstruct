#!/usr/bin/env python3
import os
import sys
import subprocess
import shutil
from pathlib import Path

# === 工具函数 ===
def print_error(msg):
    print(f"\033[91mERROR: {msg}\033[0m", file=sys.stderr)

def run_cmd(cmd, cwd=None):
    """运行命令并检查返回码"""
    env = os.environ.copy()
    result = subprocess.run(cmd, cwd=cwd, env=env)
    if result.returncode != 0:
        print_error(f"Command failed: {' '.join(map(str, cmd))}")
        sys.exit(1)

# === 配置常量 ===
PLUGIN_NAME = "Moonlight-Godot"
GODOT_CPP_PATH = Path("src/lib/godot-cpp")
MOONLIGHT_SRC = Path("src/lib/moonlight-common-c")
BUILD_ROOT = Path("build")
BIN_DIR = Path("bin")

# === 检查 godot-cpp 子模块 ===
if not (GODOT_CPP_PATH.is_dir() and any(GODOT_CPP_PATH.iterdir())):
    print_error("godot-cpp submodule not initialized. Run:\n  git submodule update --init --recursive")
    sys.exit(1)

# === 初始化 SCons 环境 ===
localEnv = Environment(tools=["default"], PLATFORM="")
customs = [Path("custom.py").resolve()] if Path("custom.py").exists() else []
opts = Variables([str(p) for p in customs], ARGUMENTS)
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

# # === Web 平台跳过处理 ===
# if platform == "web":
#     print("[Info] Web platform skipped.")
#     moonlight_built = False
# else:
moonlight_built = True
MOONLIGHT_SRC.resolve(strict=True)  # 确保源码存在

# 构建目录命名
static_build_dir = BUILD_ROOT / f"moonlight-static-{platform}-{arch}-{build_type}"
shared_build_dir = BUILD_ROOT / f"moonlight-shared-{platform}-{arch}-{build_type}"
static_build_dir.mkdir(parents=True, exist_ok=True)
shared_build_dir.mkdir(parents=True, exist_ok=True)

# 基础 CMake 参数
cmake_base_args = [
    "cmake",
    "-S", str(MOONLIGHT_SRC),
    f"-DCMAKE_BUILD_TYPE={build_type}",
]

if platform == "windows":
    cmake_base_args += ["-A", "x64" if arch == "x86_64" else "Win32"]
    cmake_base_args += ["-DCMAKE_C_FLAGS=/wd5287"]

def build_moonlight(build_dir: Path, shared: bool):
    build_type_flag = "ON" if shared else "OFF"
    args = cmake_base_args + [
        "-B", str(build_dir),
        f"-DBUILD_SHARED_LIBS={build_type_flag}",
    ]
    lib_type = "SHARED" if shared else "STATIC"
    print(f"[CMake] Configuring {lib_type} library...")
    run_cmd(args)
    print(f"[CMake] Building {lib_type} library...")
    if platform == "windows":
        run_cmd(["cmake", "--build", str(build_dir), "--config", build_type])
    else:
        run_cmd(["cmake", "--build", str(build_dir)])

# 构建静态库和共享库
build_moonlight(static_build_dir, shared=False)
build_moonlight(shared_build_dir, shared=True)

# 库文件名映射
if platform == "windows":
    static_lib = "moonlight-common-c.lib"
    shared_lib = "moonlight-common-c.dll"
    static_lib_dir = static_build_dir / build_type
    shared_lib_dir = shared_build_dir / build_type
elif platform in ("macos", "ios"):
    static_lib = "libmoonlight-common-c.a"
    shared_lib = "libmoonlight-common-c.dylib"
    static_lib_dir = static_build_dir
    shared_lib_dir = shared_build_dir
else:  # linux etc.
    static_lib = "libmoonlight-common-c.a"
    shared_lib = "libmoonlight-common-c.so"
    static_lib_dir = static_build_dir
    shared_lib_dir = shared_build_dir

static_lib_path = static_lib_dir / static_lib
shared_lib_path = shared_lib_dir / shared_lib

if not static_lib_path.is_file():
    print_error(f"Static library not found: {static_lib_path}")
    sys.exit(1)
if not shared_lib_path.is_file():
    print_error(f"Shared library not found: {shared_lib_path}")
    sys.exit(1)

# === 加载 godot-cpp SConstruct ===
env = SConscript(str(GODOT_CPP_PATH / "SConstruct"), {"env": env, "customs": [str(p) for p in customs]})

# === 添加插件源码 ===
env.Append(CPPPATH=[str(Path("src"))])
sources = Glob("src/*.cpp") + Glob("src/*/*.cpp")

# === 输出路径 ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{PLUGIN_NAME}{suffix}{env['SHLIBSUFFIX']}"
output_path = BIN_DIR / platform / plugin_filename

library = env.SharedLibrary(
    target=str(output_path),
    source=sources,
)

# === 链接与复制 Moonlight 库 ===
if moonlight_built:
    # 链接静态库（嵌入符号）
    env.Append(LIBPATH=[str(static_lib_dir)])
    env.Append(LIBS=["moonlight-common-c"])
    Depends(library, str(static_lib_path))

    # 复制共享库到插件输出目录
    plugin_bin_dir = BIN_DIR / platform
    plugin_bin_dir.mkdir(parents=True, exist_ok=True)
    dest_shared = plugin_bin_dir / shared_lib_path.name
    shutil.copy2(shared_lib_path, dest_shared)
    print(f"[Info] Copied {shared_lib_path.name} to {plugin_bin_dir}")

print(f"[Success] GDExtension built at: {output_path}")