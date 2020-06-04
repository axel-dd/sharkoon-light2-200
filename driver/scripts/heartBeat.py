#!/usr/bin/env python3

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

    # Red Heartbeat LED Mode
    data = b'\x04\xa0\x01\x02\x01\x02\xa5\x01\x07\x00\x08\x08\x00\x10\x10\x00\x18\x18\x00\x30\x30\x00\x40\x40\x00\x80\x80\x11\x40\x40\x00\x00\x00\x03\x02\x00\xa5\x08\x02\x0a\x01\x00\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    if len(data) != 64:
        print("wrong data size", file=sys.stderr)
        sys.exit(1)

    # write to test file
    print("writing to " + test_file + " ...", file=sys.stdout) 
    with open(test_file, 'wb') as test_file_writer:
        test_file_writer.write(data)

if __name__== "__main__":
    main()

        