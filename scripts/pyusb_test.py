#!/usr/bin/env python3

##
## This scripts is for playing with the device using pyusb
##

import usb.core
import usb.util

SharkoonVendorId = 0x2ea8
Light2DeviceId   = 0x2203

# switch all LEDs off
msg_led_off =      bytes.fromhex('04a001020102a50107000808001010001818003030004040008080114040000000030200a509020a010000000000000000000000000000000000000000000000')

# Red Heartbeat LED Mode
msg_heartbeat =    bytes.fromhex('04a001020102a50107000808001010001818003030004040008080114040000000030200a508020a0100ff000000000000000000000000000000000000000000')

# get mouse settings
msg_get_settings = bytes.fromhex('04a00101000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000')

# set the data to send
data = msg_led_off
if len(data) != 64:
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
    # print ("Unloading HID kernel driver for second interface...")
    dev.detach_kernel_driver(1)

endpoint_in = dev[0][(1,0)][0]
endpoint_out = dev[0][(1,0)][1]
#print ("endpoint_out",endpoint_out)
#print ("endpoint_in",endpoint_in)


# write
# print ("Write data...")
print ('=> ' + str(bytes(data).hex()))
endpoint_out.write(data)


# read
# we except two messages
# print ("Waiting to read...")
data = dev.read(endpoint_in.bEndpointAddress, 64, 500)
print ('<= ' + str(bytes(data).hex()))
# data = dev.read(endpoint_in.bEndpointAddress, 64, 500)
# print ('<= ' + str(bytes(data).hex()))
