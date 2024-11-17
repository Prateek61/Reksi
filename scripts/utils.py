import re
from art import text2art

from typing import List, Tuple

reksi_include_pattern = re.compile(r'^\s*#include\s*"Reksi/(.*\.h)\s*"')
pragma_once_pattern = re.compile(r'^\s*#pragma\s+once\s*')
pragma_region_pattern = re.compile(r'^\s*#pragma\s+region\s*')
pragma_region_defer_pattern = re.compile(r'^\s*#pragma\s+region\s+Defer\s*')
pragma_endregion_pattern = re.compile(r'^\s*#pragma\s+endregion\s*')

def is_line_pragma_region(line: str) -> bool:
    return bool(pragma_region_pattern.match(line))

def is_line_pragma_endregion(line: str) -> bool:
    return bool(pragma_endregion_pattern.match(line))

def is_line_pragma_once(line: str) -> bool:
    return bool(pragma_once_pattern.match(line))

def is_line_pragma_region_defer(line: str) -> bool:
    return bool(pragma_region_defer_pattern.match(line))

def is_reksi_header(line: str) -> bool:
    return bool(reksi_include_pattern.match(line))

def get_header_file(line: str) -> str:
    return reksi_include_pattern.match(line).group(1)

def is_line_ok_to_add(line: str) -> str:
    return not is_line_pragma_once(line) and not is_reksi_header(line)

def get_reksi_headers(file_path: str) -> List[str]:
    res: List[str] = []

    with open(file_path, 'r') as file:
        content = file.readlines()

        for line in content:
            if is_reksi_header(line):
                res.append(get_header_file(line))
    return res

def divide_into_main_and_defer(file_path: str) -> Tuple[str, str]:
    main: str = ""
    deferred: str = ""

    with open(file_path, 'r') as file:
        content = file.readlines()

        deferred_block: bool = False
        pragma_regions: int = 0

        for line in content:
            # If the line is within a #pragma region Defer block, add it to the deferred block
            if is_line_pragma_region(line) or is_line_pragma_endregion(line):
                if is_line_pragma_region_defer(line):
                    deferred_block = True
                    pragma_regions = 0
                elif is_line_pragma_region(line):
                    pragma_regions += 1
                elif is_line_pragma_endregion(line) and deferred_block == True and pragma_regions == 0:
                    deferred_block = False
                    deferred += "#pragma endregion\n"
                    continue
                elif is_line_pragma_endregion(line):
                    pragma_regions -= 1
            if is_line_ok_to_add(line):
                if deferred_block:
                    deferred += line
                else:
                    main += line

        return (main, deferred)

def get_str_art(file: str) -> str:
    # Remove .h from the file name
    file = file[:-2]
    art: str = ""
    art += "/*\n"
    art += text2art(file)
    art += "*/\n"
    return art


if __name__ == '__main__':
    main, deferred = divide_into_main_and_defer('../src/Reksi/Resource.h')
    print("------------------ Main -------------------")
    print(main)
    print("----------------- Deferred -------------------")
    print(deferred)