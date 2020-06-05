#!/usr/bin/env python3

##
## This scripts is for playing with the device using pyusb
##

import usb.core
import usb.util
import binascii

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

# write
print (f'=> {binascii.hexlify(msg)}')
endpoint_out.write(msg)

# read
# we except two messages
response = dev.read(endpoint_in.bEndpointAddress, 64, timeout=500)
print (f'<= {binascii.hexlify(response)}')
response = dev.read(endpoint_in.bEndpointAddress, 64, timeout=500)
print (f'<= {binascii.hexlify(response)}')
