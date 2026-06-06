#!/bin/env python3
# OpenCL Header to Constant Converter
# Converts OpenCL C header files to C++ string constants

import os
import sys

def convert_header_to_string(header_path):
    """Convert a C/C++ header file to a C string constant"""
    with open(header_path, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    # Escape for C string
    content = content.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n"\n"')
    return f'"{content}"'

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: header_to_string.py <input_header> <output_cpp>')
        sys.exit(1)
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    if not os.path.exists(input_path):
        print(f'Error: {input_path} not found')
        sys.exit(1)
    content = convert_header_to_string(input_path)
    with open(output_path, 'w') as f:
        f.write(f'// Auto-generated from {input_path}\n')
        f.write(f'const char* HEADER_CONTENT = \n{content};\n')
    print(f'Converted {input_path} -> {output_path}')
