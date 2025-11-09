#!/usr/bin/env python
import os
import sys
import subprocess
from pathlib import Path

from methods import print_error

# === 插件配置 ===
libname = "Moonlight-Godot"        # ← 修改为你想要的插件名
projectdir = "demo"          # demo 项目目录（用于复制）

# === 初始化环境 ===
localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs if os.path.exists(path)]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

# === 检查 godot-cpp 子模块 ===
godot_cpp_path = "src/lib/godot-cpp"
if not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path)):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

# === 构建 moonlight-common-c via CMake ===
# platform = env["platform"]
# arch = env["arch"]
# target = env["target"]


platform = ARGUMENTS.get("platform", "linux")
arch = ARGUMENTS.get("arch", "x86_64")
target = ARGUMENTS.get("target", "template_debug")
precision = ARGUMENTS.get("precision", "single")
use_mbedtls = ARGUMENTS.get("use_mbedtls", "false").lower() == "true"
is_debug = target in ("editor", "template_debug")
build_type = "Debug" if is_debug else "Release"

moonlight_src = "src/lib/moonlight-common-c"
build_root = "build"
moonlight_build_dir = f"{build_root}/moonlight-{platform}-{arch}-{build_type}"

Path(moonlight_build_dir).mkdir(parents=True, exist_ok=True)

# CMake 配置
cmake_args = [
    "cmake",
    "-S", moonlight_src,
    "-B", moonlight_build_dir,
    f"-DCMAKE_BUILD_TYPE={build_type}",
    "-DBUILD_SHARED_LIBS=OFF",   # ←←← 强制静态库
]

if platform == "windows":
    cmake_args += ["-A", "x64" if arch == "x86_64" else "Win32"]
    cmake_args += ["-DCMAKE_C_FLAGS=/wd5287"]  # 抑制 ENet 枚举警告

print(f"[CMake] Configuring moonlight-common-c ({platform} {arch} {build_type})...")
ret = subprocess.run(cmake_args, cwd=".", env=os.environ)
if ret.returncode != 0:
    sys.exit(f"CMake configure failed with code {ret.returncode}")

# CMake 构建
print("[CMake] Building moonlight-common-c...")
ret = subprocess.run(["cmake", "--build", moonlight_build_dir, "--config", build_type], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"CMake build failed with code {ret.returncode}")

# === 确定 moonlight 库路径 ===
if platform == "windows":
    moonlight_lib_dir = os.path.join(moonlight_build_dir, build_type)  # e.g., Debug/
    moonlight_lib_name = "moonlight-common-c.lib"
    moonlight_dll_name = "moonlight-common-c.dll"
else:
    moonlight_lib_dir = os.path.join(moonlight_build_dir, build_type) 
    moonlight_lib_name = "moonlight-common-c" + (".dylib" if platform == "macos" else ".so")
    moonlight_dll_name = moonlight_lib_name

# === 加载 godot-cpp 构建环境 ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

# === 添加源码和头文件路径 ===
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))
sources.extend(Glob("src/*/*/*.cpp"))
sources.extend(Glob("src/*/*/*/*.cpp"))

# === 添加 doc_data（可选）===
if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        pass  # Pre-4.3 Godot doesn't have this

# === 链接 moonlight 库 ===
env.Append(LIBPATH=[moonlight_lib_dir])
if platform == "windows":
    env.Append(LIBS=["moonlight-common-c"])
else:
    env.Append(LIBS=["moonlight-common-c"])

# === 构建 GDExtension 共享库 ===
# Godot 插件命名规范: <name>.<platform>.<arch>.<debug/release>.<ext>
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{env['platform']}/{plugin_filename}"

library = env.SharedLibrary(
    target=output_path,
    source=sources,
)

# === 依赖：确保 moonlight 库先构建 ===
moonlight_lib_full = os.path.join(moonlight_lib_dir, moonlight_lib_name)
Depends(library, moonlight_lib_full)

# === Windows: 复制运行时 DLL 到 bin/windows/ ===
if platform == "windows":
    moonlight_dll_src = os.path.join(moonlight_lib_dir, moonlight_dll_name)
    moonlight_dll_dst = f"bin/{env['platform']}/{moonlight_dll_name}"
    copy_dll = env.Command(moonlight_dll_dst, moonlight_dll_src, Copy("$TARGET", "$SOURCE"))
    Default(copy_dll)

# === 复制到 demo 项目（可选）===
demo_copy = env.Install(f"{projectdir}/bin/{env['platform']}", library)
if platform == "windows":
    demo_copy_dll = env.Install(f"{projectdir}/bin/{env['platform']}", moonlight_dll_dst)
    Default(demo_copy, demo_copy_dll)
else:
    Default(demo_copy)

# === 默认目标 ===
Default(library, demo_copy)