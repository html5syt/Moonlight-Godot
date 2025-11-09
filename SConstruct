#!/usr/bin/env python
import os
import sys
import subprocess
import shutil
from pathlib import Path

from methods import print_error

# import io

# sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')
# sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding='utf-8')

# === 插件配置 ===
libname = "Moonlight-Godot"
projectdir = "demo"

localEnv = Environment(tools=["default"], PLATFORM="")

customs = ["custom.py"]
customs = [os.path.abspath(path) for path in customs if os.path.exists(path)]

opts = Variables(customs, ARGUMENTS)
opts.Update(localEnv)

Help(opts.GenerateHelpText(localEnv))

env = localEnv.Clone()

godot_cpp_path = "src/lib/godot-cpp"
if not (os.path.isdir(godot_cpp_path) and os.listdir(godot_cpp_path)):
    print_error("""godot-cpp is not available within this folder, as Git submodules haven't been initialized.
Run the following command to download godot-cpp:

    git submodule update --init --recursive""")
    sys.exit(1)

# === 解析参数 ===
platform = ARGUMENTS.get("platform", "linux")
arch = ARGUMENTS.get("arch", "x86_64")
target = ARGUMENTS.get("target", "template_debug")
is_debug = target in ("editor", "template_debug")
build_type = "Debug" if is_debug else "Release"

moonlight_src = Path("src/lib/moonlight-common-c").resolve()
build_root = "build"
moonlight_build_dir = f"{build_root}/moonlight-{platform}-{arch}-{build_type}"
Path(moonlight_build_dir).mkdir(parents=True, exist_ok=True)

cmake_lists = moonlight_src / "CMakeLists.txt"
original_cmake_lists = moonlight_src / "CMakeLists.txt.original"

# === 仅在 Windows 上需要 patch：在 if(MSVC) 后插入导出符号设置 ===
if platform == "windows":
    cmake_lists = moonlight_src / "CMakeLists.txt"
    original_cmake_lists = moonlight_src / "CMakeLists.txt.original"

    # 备份原始文件（仅首次）
    if not original_cmake_lists.exists():
        shutil.copy(cmake_lists, original_cmake_lists)

    # 读取原始内容
    with open(original_cmake_lists, "r", encoding="utf-8") as f:
        lines = f.readlines()

    patched = False
    new_lines = []
    inserted = False

    for line in lines:
        new_lines.append(line)
        # 检查是否是 if(MSVC) 行（忽略前后空格）
        stripped = line.strip()
        if not inserted and stripped.startswith("if(MSVC)") and not stripped.startswith("if(MSVC OR"):
            # 插入导出符号设置
            new_lines.append("    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)\n")
            inserted = True
            patched = True

    # 如果没找到 if(MSVC)，追加到文件末尾（兜底）
    if not patched:
        new_lines.append("\n# Patched by SConstruct: enable symbol export on Windows\n")
        new_lines.append("if(MSVC)\n")
        new_lines.append("    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)\n")
        new_lines.append("endif()\n")

    # 写回 CMakeLists.txt
    with open(cmake_lists, "w", encoding="utf-8") as f:
        f.writelines(new_lines)

# === CMake 配置（现在可以安全启用 SHARED）===
cmake_args = [
    "cmake",
    "-S", str(moonlight_src),
    "-B", moonlight_build_dir,
    f"-DCMAKE_BUILD_TYPE={build_type}",
]

if platform == "windows":
    cmake_args += ["-A", "x64" if arch == "x86_64" else "Win32"]
    cmake_args += ["-DCMAKE_C_FLAGS=/wd5287"]  # 抑制 ENet 警告
    # 关键：允许 CMake 自动导出所有符号 → 生成 .lib
    cmake_args += ["-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=ON"]

print(f"[CMake] Configuring moonlight-common-c as SHARED library ({platform} {arch} {build_type})...")
ret = subprocess.run(cmake_args, cwd=".", env=os.environ)
if ret.returncode != 0:
    sys.exit(f"CMake configure failed with code {ret.returncode}")

print("[CMake] Building moonlight-common-c (shared)...")
ret = subprocess.run(["cmake", "--build", moonlight_build_dir, "--config", build_type], env=os.environ)
if ret.returncode != 0:
    sys.exit(f"CMake build failed with code {ret.returncode}")

# === 确定输出路径（Windows: .dll + .lib）===
if platform == "windows":
    lib_dir = os.path.join(moonlight_build_dir, build_type)
    moonlight_lib_name = "moonlight-common-c.lib"
    moonlight_dll_name = "moonlight-common-c.dll"
else:
    lib_dir = moonlight_build_dir
    moonlight_lib_name = "libmoonlight-common-c.a"
    moonlight_dll_name = None

moonlight_lib_full = os.path.join(lib_dir, moonlight_lib_name)
if not os.path.isfile(moonlight_lib_full):
    sys.exit(f"ERROR: Required library not found: {moonlight_lib_full}")

# === 加载 godot-cpp ===
env = SConscript(os.path.join(godot_cpp_path, "SConstruct"), {"env": env, "customs": customs})

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")
sources.extend(Glob("src/*/*.cpp"))
sources.extend(Glob("src/*/*/*.cpp"))
sources.extend(Glob("src/*/*/*/*.cpp"))

if env["target"] in ["editor", "template_debug"]:
    try:
        doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
        sources.append(doc_data)
    except AttributeError:
        pass

# === 链接 ===
env.Append(LIBPATH=[lib_dir])
env.Append(LIBS=["moonlight-common-c"])

# === 构建插件 ===
suffix = env["suffix"].replace(".dev", "").replace(".universal", "")
plugin_filename = f"{libname}{suffix}{env['SHLIBSUFFIX']}"
output_path = f"bin/{env['platform']}/{plugin_filename}"

library = env.SharedLibrary(
    target=output_path,
    source=sources,
)

Depends(library, moonlight_lib_full)

# === Windows: 复制 DLL 到 bin ===
if platform == "windows":
    dll_src = os.path.join(lib_dir, moonlight_dll_name)
    dll_dst = f"bin/{env['platform']}/{moonlight_dll_name}"
    copy_dll = env.Command(dll_dst, dll_src, Copy("$TARGET", "$SOURCE"))
    demo_copy_dll = env.Install(f"{projectdir}/bin/{env['platform']}", dll_dst)
    Default(copy_dll, demo_copy_dll)

# === 复制到 demo ===
demo_copy = env.Install(f"{projectdir}/bin/{env['platform']}", library)
Default(library, demo_copy)