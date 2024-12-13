import os
from utils import *
from art import *

# Paths
PROJECT_BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REKSI_H_FILE = os.path.join(PROJECT_BASE, 'Reksi.h')
REKSI_INCLUDE_FILE = os.path.join(PROJECT_BASE, 'src', "ReksiInclude.h")
REKSI_HEADERS_DIR = os.path.join(PROJECT_BASE, 'src', 'Reksi')

def main():
    reksi_headers = get_reksi_headers(REKSI_INCLUDE_FILE)

    data: List[Tuple[str, str]] = []
    for header in reksi_headers:
        main, deferred = divide_into_main_and_defer(os.path.join(REKSI_HEADERS_DIR, header))
        data.append((main, deferred))
    
    # First, clear the contents of the Reksi.h file
    with open(REKSI_H_FILE, 'w') as file:
        file.write("#pragma once\n\n")

    # Write contents of datas main definitions
    with open(REKSI_H_FILE, 'a') as file:
        for entry, header in zip(data, reksi_headers):
            file.write(get_str_art(header))
            file.write(entry[0])
            file.write("\n\n")

    # Write contents of datas deferred definitions
    with open(REKSI_H_FILE, 'a') as file:
        file.write(get_str_art("Definitions.h"))
        file.write("\n")
        for _, deferred_code in data:
            # If deferred code is not just whitespace or newlines then add it to the Reksi.h file
            if deferred_code:
                file.write(deferred_code)
                file.write("\n\n")

if __name__ == '__main__':
    main()