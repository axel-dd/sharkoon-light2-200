#!/usr/bin/env python3

##
## This scripts is for playing with the device using pyusb
##

import usb.core
import binascii
import threading
import time
import errno

SharkoonVendorId = 0x2ea8
Light2DeviceId   = 0x2203

# get mouse settings
msg = binascii.unhexlify('04a00101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000')

# check package size
if len(msg) != 64:
    raise Exception('Wrong data size')

# find our device
dev = usb.core.find(idVendor=SharkoonVendorId, idProduct=Light2DeviceId)

# was it found?
if dev is None:
    raise ValueError('Device not found')

# we must unload the HID kernel driver associated with the second interface
# reattach is not necessary because the second interface does not transfer HID data
# https://stackoverflow.com/questions/29345325/raspberry-pyusb-gets-resource-busy
if dev.is_kernel_driver_active(1):
    dev.detach_kernel_driver(1)

endpoint_in = dev[0][(1,0)][0]
endpoint_out = dev[0][(1,0)][1]

do_read = True

def read_usb():
    # read unit do_read become False
    while do_read:
        try:
            response = dev.read(endpoint_in.bEndpointAddress, 64)
        except usb.core.USBError as e:
            if (e.errno != errno.ETIMEDOUT):
                raise
        else:
            print (f'<= {binascii.hexlify(response)}')

# start reading in a thread
read_thread = threading.Thread(target=read_usb)
read_thread.start()

# write
print (f'=> {binascii.hexlify(msg)}')
endpoint_out.write(msg)

# sleep for 1 second
time.sleep(1)

# end reading
do_read = False
read_thread.join()