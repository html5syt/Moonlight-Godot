import os
import sys
import platform
import shutil
import requests
import zipfile
import tarfile
import tempfile
from pathlib import Path
import time 


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
    
    print(f"Warning: If platform parameters are not specified, use automatic detection: {system}/{machine}")
    
    # 以下为简化的自动检测部分
    os_str = 'win' if system == 'windows' else ('linux' if system == 'linux' else '')
    arch_str = '64' if machine in ['amd64', 'x86_64'] else ('arm64' if machine in ['aarch64', 'arm64'] else '')

    if os_str and arch_str:
        return f"{os_str}{arch_str}"
    
    print(f"Error: Cannot automatically determine platform. Please specify manually (e.g. win64)")
    sys.exit(1)




def get_download_url(platform_key):
    """
    从 GitHub API 获取对应平台的下载链接，并在失败时无限次、间隔 5 秒重试。
    """
    print(f"The latest Release information is being obtained (Target platform:{platform_key})...")

    # === 新增重试逻辑 ===
    while True:
        try:
            # 尝试发送请求
            response = requests.get(GITHUB_API_URL)
            
            # 检查 HTTP 状态码 (200, 300, 400, 500 等)
            response.raise_for_status() 
            
            # 如果请求成功且状态码在 200-300 之间，解析 JSON
            data = response.json()
            
            # 成功获取并解析，跳出重试循环
            break 

        except requests.exceptions.RequestException as e:
            # 捕获所有由 requests 引起的错误，包括网络连接问题和非 2xx 状态码
            print(f"Failed to obtain API (network or HTTP error) : {e}. Try again in 5 seconds...")
            time.sleep(5)
            continue # 继续下一次循环重试
        except Exception as e:
            # 捕获其他非网络错误，例如 JSON 解析错误
            print(f"Failed to parse API response: {e}. Try again in 5 seconds...")
            time.sleep(5)
            continue # 继续下一次循环重试
    # === 结束重试逻辑 ===


    # 目标文件名特征: ffmpeg-master-latest-{platform}-gpl-shared
    search_str = f"ffmpeg-master-latest-{platform_key}-gpl-shared"
    
    for asset in data.get('assets', []):
        name = asset['name']
        if search_str in name:
            print(f"Find the matching resource: {name}")
            return asset['browser_download_url'], name
            
    print(f"Error: The master shared version matching the {platform_key} platform was not found.")
    sys.exit(1)

def download_file(url, filename):
    """下载文件并显示进度"""
    print(f"Start downloading {filename} ...")
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
                        # done = int(50 * dl / total_length)
                        # sys.stdout.write(f"\r[{'=' * done}{' ' * (50-done)}] {dl/1024/1024:.2f} MB")
                        # sys.stdout.flush()
    print("\nDownload completed.")

def extract_and_install(archive_path, target_output_dir):
    """解压并移动 include 和 lib 文件夹到指定的 target_output_dir"""
    temp_dir = Path(tempfile.mkdtemp())
    target_path = Path(target_output_dir) # 使用传入的参数作为目标路径
    print(f"Decompressing to temporary directory: {temp_dir} ...")

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
            print("Error: The root directory structure cannot be found in the compressed package.")
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
                print(f"Move {folder} -> {dst_path}")
                shutil.copytree(src_path, dst_path)
            else:
                print(f"Warning: The {folder} folder is not found in the compressed package.")

        print(f"Decompression and installation completed, files are located in: {target_path.absolute()}")

    except Exception as e:
        print(f"Decompression error: {e}")
    finally:
        # 清理临时目录
        shutil.rmtree(temp_dir)

def main():
    # 检查命令行参数数量
    if len(sys.argv) < 3:
        print("Usage: python ffmpeg-down.py <platform_key> <target_output_dir>")
        print("Example: python ffmpeg-down.py win64 lib/ffmpeg/win64")
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
            print("Cleaning up downloaded archive...")
            os.remove(filename)

if __name__ == "__main__":
    main()