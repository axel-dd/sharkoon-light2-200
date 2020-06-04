#!/usr/bin/env python3

##
## This scripts updates the kernel and gcc include paths in c_cpp_properties.json
##

import re
import sys
import errno
import os
from pathlib import Path
import platform
import glob

def main():
    kernel_modules_dir = Path('/lib/modules').joinpath(platform.release())
    kernel_h = kernel_modules_dir.joinpath('build/include/linux/kernel.h')
    if not kernel_h.exists():
        print("ERROR: kernel headers could not be found.")
        sys.exit(1)

    gcc_include_dirs = glob.glob('/usr/lib/gcc/*/*/include')
    if not gcc_include_dirs:
        print("ERROR: gcc headers could not be found.")
        sys.exit(1)

    fin = Path(__file__).parent.joinpath('c_cpp_properties.json').open('rt')
    content = fin.read()
    fin.close()

    content = re.sub(r'\/lib\/modules\/[^\/]*',
                        str(kernel_modules_dir),
                        content)

    content = re.sub(r'\/usr\/lib\/gcc\/[^\/]*\/[^\/]*\/include',
                        gcc_include_dirs[0],
                        content)

    fout = Path(__file__).parent.joinpath('c_cpp_properties.json').open('wt')
    fout.write(content)    
    fout.close()

if __name__ == '__main__':
    main()