from pathlib import Path
import format_include
import os
import subprocess

def format_files(directory: Path, local_include_directory: Path):
    for root, dirs, files in os.walk(directory):
        for file in files:
            for extension in format_include.EXTENSIONS:
                if file.endswith(extension):
                    break
            else:
                continue

            file_path: Path = Path(os.path.join(root, file))

            print(f'[{local_include_directory}] Formatting {file_path}')
            format_include.format_file(file_path, local_include_directory)

            subprocess.run(["clang-format", "--style=file:.clang-format", "-i", file_path])

if __name__ == "__main__":
    format_files(Path("Karipon/include/"), Path("Karipon/include/"))
    format_files(Path("Karipon/source/"), Path("Karipon/include/"))
