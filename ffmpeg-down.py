import os
import sys
import platform
import shutil
import requests
import zipfile
import tarfile
import tempfile
from pathlib import Path

# GITHUB API URL 保持不变
GITHUB_API_URL = "https://api.github.com/repos/BtbN/FFmpeg-Builds/releases/tags/latest"

def get_platform_key(target_platform=None):
    """
    获取平台标识符 (如 win64, linux64)。
    在 CI 环境中，我们倾向于直接使用传入的 target_platform。
    """
    if target_platform:
        return target_platform

    # 如果没有传入参数（例如本地运行），则尝试自动检测
    system = platform.system().lower()
    machine = platform.machine().lower()

    # ... (此处省略了自动检测逻辑，因为在 CI 中会传入参数)
    # ... (如果您需要完整的自动检测逻辑，请保留原脚本中的代码)
    
    print(f"警告: 未指定平台参数，使用自动检测: {system}/{machine}")
    
    # 以下为简化的自动检测部分
    os_str = 'win' if system == 'windows' else ('linux' if system == 'linux' else '')
    arch_str = '64' if machine in ['amd64', 'x86_64'] else ('arm64' if machine in ['aarch64', 'arm64'] else '')

    if os_str and arch_str:
        return f"{os_str}{arch_str}"
    
    print(f"错误: 无法自动确定平台。请手动指定 (如 win64)")
    sys.exit(1)


def get_download_url(platform_key):
    """
    从 GitHub API 获取对应平台的下载链接。
    """
    print(f"正在获取最新 Release 信息 (目标平台: {platform_key})...")
    try:
        response = requests.get(GITHUB_API_URL)
        response.raise_for_status()
        data = response.json()
    except Exception as e:
        print(f"获取 API 失败: {e}")
        sys.exit(1)

    # 目标文件名特征: ffmpeg-master-latest-{platform}-gpl-shared
    # 保持原脚本逻辑，搜索 shared 版本
    search_str = f"ffmpeg-master-latest-{platform_key}-gpl-shared"
    
    for asset in data.get('assets', []):
        name = asset['name']
        if search_str in name:
            print(f"找到匹配资源: {name}")
            return asset['browser_download_url'], name
            
    print(f"错误: 未找到匹配 {platform_key} 平台的 master shared 版本。")
    sys.exit(1)

def download_file(url, filename):
    """下载文件并显示进度"""
    print(f"开始下载: {filename} ...")
    with requests.get(url, stream=True) as r:
        r.raise_for_status()
        total_length = int(r.headers.get('content-length', 0))
        with open(filename, 'wb') as f:
            if total_length == 0:
                f.write(r.content)
            else:
                dl = 0
                for chunk in r.iter_content(chunk_size=8192):
                    if chunk:
                        dl += len(chunk)
                        f.write(chunk)
                        done = int(50 * dl / total_length)
                        sys.stdout.write(f"\r[{'=' * done}{' ' * (50-done)}] {dl/1024/1024:.2f} MB")
                        sys.stdout.flush()
    print("\n下载完成。")

def extract_and_install(archive_path, target_output_dir):
    """解压并移动 include 和 lib 文件夹到指定的 target_output_dir"""
    temp_dir = Path(tempfile.mkdtemp())
    target_path = Path(target_output_dir) # 使用传入的参数作为目标路径
    print(f"正在解压到临时目录: {temp_dir} ...")

    try:
        # 1. 解压
        if archive_path.endswith('.zip'):
            with zipfile.ZipFile(archive_path, 'r') as zf:
                zf.extractall(temp_dir)
        elif archive_path.endswith('.tar.xz'):
            with tarfile.open(archive_path, 'r:xz') as tf:
                tf.extractall(temp_dir)
        
        # 2. 寻找解压后的根目录
        extracted_root = None
        for item in temp_dir.iterdir():
            if item.is_dir() and item.name.startswith('ffmpeg-master'):
                extracted_root = item
                break
        
        if not extracted_root:
            print("错误: 无法在压缩包中找到根目录结构。")
            return

        # 3. 移动 include 和 lib
        # 确保目标路径存在，如果存在则先清空
        if target_path.exists():
            shutil.rmtree(target_path)
        target_path.mkdir(parents=True, exist_ok=True)

        target_folders = ['include', 'lib', "bin"]
        
        for folder in target_folders:
            src_path = extracted_root / folder
            dst_path = target_path / folder
            
            if src_path.exists():
                print(f"移动 {folder} -> {dst_path}")
                shutil.copytree(src_path, dst_path)
            else:
                print(f"警告: 压缩包中未找到 {folder} 文件夹")

        print(f"解压安装完成，文件位于: {target_path.absolute()}")

    except Exception as e:
        print(f"解压过程中出错: {e}")
    finally:
        # 清理临时目录
        shutil.rmtree(temp_dir)

def main():
    # 检查命令行参数数量
    if len(sys.argv) < 3:
        print("用法: python ffmpeg-down.py <platform_key> <target_output_dir>")
        print("示例: python ffmpeg-down.py win64 lib/ffmpeg/win64")
        sys.exit(1)
        
    manual_platform = sys.argv[1] # 第一个参数是平台 key
    target_output_dir = sys.argv[2] # 第二个参数是目标输出目录
    
    platform_key = get_platform_key(manual_platform)
    url, filename = get_download_url(platform_key)
    
    try:
        download_file(url, filename)
        # 传入目标路径给解压函数
        extract_and_install(filename, target_output_dir) 
    finally:
        # 删除下载的压缩包
        if os.path.exists(filename):
            print("正在清理下载的压缩包...")
            os.remove(filename)

if __name__ == "__main__":
    main()