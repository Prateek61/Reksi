import os
import re

from typing import List

# Paths
PROJECT_BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REKSI_H_FILE = os.path.join(PROJECT_BASE, 'Reksi.h')
REKSI_INCLUDE_FILE = os.path.join(PROJECT_BASE, 'src', "ReksiInclude.h")
REKSI_HEADERS_DIR = os.path.join(PROJECT_BASE, 'src', 'Reksi')

reksi_include_pattern = re.compile(r'^\s*#include\s*"Reksi/(.*\.h)\s*"')
pragma_once_pattern = re.compile(r'^\s*#pragma\s+once\s*')

def is_pragma_once(line: str) -> bool:
    return bool(pragma_once_pattern.match(line))

def is_reksi_header(line: str) -> bool:
    return bool(reksi_include_pattern.match(line))

def is_line_ok(line: str) -> bool:
    return not is_pragma_once(line) and not is_reksi_header(line)

def get_header_file(line: str) -> str:
    return reksi_include_pattern.match(line).group(1)

def get_reksi_headers(file_path: str) -> List[str]:
    res: List[str] = []

    with open(file_path, 'r') as file:
        content = file.readlines()

        for line in content:
            if is_reksi_header(line):
                res.append(get_header_file(line))

    return res
    

def get_sanitated_file(file_path: str) -> str:
    result = ""

    with open(file_path, 'r') as file:
        content = file.readlines()

        for line in content:
            if is_line_ok(line):
                result += line

    return result


def main():
    reksi_headers = get_reksi_headers(REKSI_INCLUDE_FILE)

    sanitized_files: List[str] = []
    for header in reksi_headers:
        file_path = os.path.join(REKSI_HEADERS_DIR, header)
        sanitized_files.append(get_sanitated_file(file_path))

    with open(REKSI_H_FILE, 'w') as file:
        file.write("#pragma once\n")

        for sanitized_file, header in zip(sanitized_files, reksi_headers):
            file.write(f"\n// -------------- Start of {header}----------------\n")
            file.write(sanitized_file)
            file.write(f"\n// -------------------- End of {header}------------------\n\n")


if __name__ == "__main__":
    main()