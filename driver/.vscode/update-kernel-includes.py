#!/usr/bin/python3

##
## This scripts updates the kernel include paths in c_cpp_properties.json
##

import re
import errno
import os
from pathlib import Path
import platform

def main():
    kernel_modules_dir = Path('/lib/modules').joinpath(platform.release())
    kernel_h = kernel_modules_dir.joinpath('build/include/linux/kernel.h')
    if not kernel_h.exists():
        print("ERROR: kernel headers could not be found.")
        return

    fin = Path(__file__).parent.joinpath('c_cpp_properties.json').open('rt')
    content = fin.read()
    fin.close()

    content = re.sub(r'\/lib\/modules\/[^\/]*',
                     str(kernel_modules_dir),
                     content)

    fout = Path(__file__).parent.joinpath('c_cpp_properties.json').open('wt')
    fout.write(content)    
    fout.close()

if __name__== "__main__":
    main()