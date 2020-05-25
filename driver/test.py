#!/usr/bin/python3

##
## This scripts helps to test the sysfs device attribute test
##

import glob
import os
import sys
import time

def main():
    test_files = glob.glob('/sys/bus/hid/drivers/sharkoon-light2-200/*:*:*.*/test')

    if len(test_files) > 1:
        print("Multiple mouse directories found.", file=sys.stderr)
        sys.exit(1)
    if len(test_files) < 1:
        print("No mouse directories found. Make sure the driver is binded", file=sys.stderr)
        sys.exit(1)

    test_file = test_files[0]

    # write to test file
    print("writing to " + test_file + " ...", file=sys.stdout) 
    with open(test_file, 'wb') as test_file_writer:
        # test_file_writer.write(b'\x00\x00')
        test_file_writer.write(b'\x04\xA0\x01\x01') # request the mouse settings
        time.sleep(1)

    # read test file (read response)
    print("reading from " + test_file, file=sys.stdout) 
    os.system("cat " + test_file)

if __name__== "__main__":
    main()