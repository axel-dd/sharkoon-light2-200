# Sharkoon Light2 200 usb message protocol

The mouse sends/receives messages over the second usb interface.
For more details, see [usb device details](#usb-device-details)

The size of in/out messages are 64 bytes

## Message Protocol

### Get settings from device

#### Send to device

| Bytes | Description | Value |
|---|---|---|
| 1 | Version number | 0x04 |
| 2-3 | Message type? | 0xa001 |
| 4 | Get settings command | 0x01 |
| 5-64 | Zero
 
#### Receive from device

| Bytes | Description | Value |
|---|---|---|
| 1 | Version number | 0x04 |
| 2-3 | Message type? | 0xa001 |
| 4 | Get settings command | 0x01 |
| 5-7 | Unknown | 0x0102a5 |
| 8 | DPI step number of the device | 0x00 - 0x06 |
| 9 | [ON/OFF state of DPI steps](#DPI-steps-encoding) | e.g. 0x7f |
| 10-12 | [DPI1 settings](#DPI-values-encoding) | e.g. 0x000808 |
| 13-15 | [DPI2 settings](#DPI-values-encoding) | e.g. 0x001010 |
| 16-18 | [DPI3 settings](#DPI-values-encoding) | e.g. 0x001818 |
| 19-21 | [DPI4 settings](#DPI-values-encoding) | e.g. 0x003030 |
| 22-24 | [DPI5 settings](#DPI-values-encoding) | e.g. 0x004040 |
| 25-27 | [DPI6 settings](#DPI-values-encoding) | e.g. 0x114040 |
| 28-30 | [DPI7 settings](#DPI-values-encoding) | e.g. 0x114040 |
| 31-33 | Unknown / Zero
| 34 | [Lift-Off distance](#Lift-Off-distance) | 0x02 - 0x04 |
| 35-37 | Unknown | 0x0200a5 |
| 38 | [LED effect](#LED-effects) | 0x00 - 0x09 |
| 39 | LED frequency | 0x00 - 0x02 (reversed order, the lower the faster) |
| 40 | LED brightness | 0x00 - 0x0a (the higher the brighter) |
| 41 | Unknown | mostly 0x01 |
| 42 | Settings Profile ID | 0x01 - 0x05 |
| 43-45 | Color1 (RGB) | 0x000000 |
| 46-48 | Color2 (RGB) | 0x000000 |
| 49-51 | Color3 (RGB) | 0x000000 |
| 52-54 | Color4 (RGB) | 0x000000 |
| 55-57 | Color5 (RGB) | 0x000000 |
| 58-60 | Color6 (RGB) | 0x000000 |
| 61-63 | Color7 (RGB) | 0x000000 |
| 64 | Unknown / Zero

### Send new settings to the device

#### Send to device

| Bytes | Description | Value |
|---|---|---|
| 1 | Version number | 0x04 |
| 2-3 | Message type? | 0xa001 |
| 4 | Set settings command | 0x02 |
| 5-7 | Unknown | 0x0102a5 |
| 8 | DPI step number of the device | 0x00 - 0x06 |
| 9 | [ON/OFF state of DPI steps](#DPI-steps-encoding) | e.g. 0x7f |
| 10-12 | [DPI1 settings](#DPI-values-encoding) | e.g. 0x000808 |
| 13-15 | [DPI2 settings](#DPI-values-encoding) | e.g. 0x001010 |
| 16-18 | [DPI3 settings](#DPI-values-encoding) | e.g. 0x001818 |
| 19-21 | [DPI4 settings](#DPI-values-encoding) | e.g. 0x003030 |
| 22-24 | [DPI5 settings](#DPI-values-encoding) | e.g. 0x004040 |
| 25-27 | [DPI6 settings](#DPI-values-encoding) | e.g. 0x114040 |
| 28-30 | [DPI7 settings](#DPI-values-encoding) | e.g. 0x114040 |
| 31-33 | Unknown / Zero
| 34 | [Lift-Off distance](#Lift-Off-distance) | 0x02 - 0x04 |
| 35-37 | Unknown | 0x0200a5 |
| 38 | [LED effect](#LED-effects) | 0x00 - 0x09 |
| 39 | LED frequency | 0x00 - 0x02 (reversed order, the lower the faster) |
| 40 | LED brightness | 0x00 - 0x0a (the higher the brighter) |
| 41 | Unknown | mostly 0x01 |
| 42 | Settings Profile ID | 0x01 - 0x05 |
| 43-45 | Color1 (RGB) | 0x000000 |
| 46-48 | Color2 (RGB) | 0x000000 |
| 49-51 | Color3 (RGB) | 0x000000 |
| 52-54 | Color4 (RGB) | 0x000000 |
| 55-57 | Color5 (RGB) | 0x000000 |
| 58-60 | Color6 (RGB) | 0x000000 |
| 61-63 | Color7 (RGB) | 0x000000 |
| 64 | Unknown / Zero

#### Receive from device

The same message data that was sent.

### DPI changed by mouse button

#### Receive from device

| Bytes | Description | Value |
|---|---|---|
| 1 | Version number | 0x04 |
| 2-3 | Message type? | 0xa202 |
| 4 | DPI step number of the device | 0x00 - 0x06 |
| 5-64 | Zero


## DPI steps encoding

The ON/OFF state of the seven DPI steps is encoded in a bit mask of one byte.

| Bit | Description | Value |
| --- | --- | --- |
| 1 | DPI1 | 0=OFF 1=ON |
| 2 | DPI2 | 0=OFF 1=ON |
| 3 | DPI3 | 0=OFF 1=ON |
| 4 | DPI4 | 0=OFF 1=ON |
| 5 | DPI5 | 0=OFF 1=ON |
| 6 | DPI6 | 0=OFF 1=ON |
| 7 | DPI7 | 0=OFF 1=ON |
| 8 | - | 0



## DPI values encoding

DPI for X-axis and Y-axis is stored in 3 bytes. The device supports regular values between 50 and 16000.  DPI values can only be set and stored in steps of 50. For the byte interpretation the regular DPI value from the UI must be divided by 50. 

The protocol data value for X-axis is stored in the first four bits of the first byte and the second byte. The protocol data value for Y-axis is stored in the last four bits of the first byte and the third byte.

### Sample

| Data | X | Y |
|---|---|---|
| 0x01f02c | 0x0f2 | 0x12c |
| | 240 | 300 |
| x 50 | 12000 DPI | 15000 DPI |

## Lift-Off distance

| UI value | Byte value |
|---|---|
| 1 | 0x02 |
| 2 | 0x03 |
| 3 | 0x04 |

## LED effects

| LED effect | Byte value | More settings supported |
|---|---|---|
| Pulsating RGB Cycle | 0x00 | Frequency, Brightness |
| Pulsating | 0x01 | Frequency, Brightness, Color1 |
| Permanent | 0x02 | Brightness, Color1 |
| Color Change | 0x03 | Frequency, Brightness, Color1 |
| Single Color Marquee | 0x04 | Frequency, Brightness, Color1 |
| Multi Color Marquee | 0x05 | Frequency, Brightness, Color1 - Color7 |
| Ripple Effect | 0x06 | Frequency, Brightness |
| Trigger | 0x07 | Frequency, Brightness, Color1 - Color7 |
| Heartbeat | 0x08 | Frequency, Brightness, Color1 |
| LED OFF | 0x09 | none |

## USB device details

    Bus 003 Device 006: ID 2ea8:2203 Sharkoon Technologies GmbH Gaming Mouse
    Device Descriptor:
    bLength                18
    bDescriptorType         1
    bcdUSB               1.10
    bDeviceClass            0 
    bDeviceSubClass         0 
    bDeviceProtocol         0 
    bMaxPacketSize0        64
    idVendor           0x2ea8 
    idProduct          0x2203 
    bcdDevice            1.04
    iManufacturer           1 Sharkoon Technologies GmbH
    iProduct                2 Gaming Mouse
    iSerial                 0 
    bNumConfigurations      1
    Configuration Descriptor:
        bLength                 9
        bDescriptorType         2
        wTotalLength       0x0042
        bNumInterfaces          2
        bConfigurationValue     1
        iConfiguration          0 
        bmAttributes         0xa0
        (Bus Powered)
        Remote Wakeup
        MaxPower              480mA
        Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        0
        bAlternateSetting       0
        bNumEndpoints           1
        bInterfaceClass         3 Human Interface Device
        bInterfaceSubClass      1 Boot Interface Subclass
        bInterfaceProtocol      2 Mouse
        iInterface              0 
            HID Device Descriptor:
            bLength                 9
            bDescriptorType        33
            bcdHID               1.11
            bCountryCode            0 Not supported
            bNumDescriptors         1
            bDescriptorType        34 Report
            wDescriptorLength      81
            Report Descriptors: 
            ** UNAVAILABLE **
        Endpoint Descriptor:
            bLength                 7
            bDescriptorType         5
            bEndpointAddress     0x81  EP 1 IN
            bmAttributes            3
            Transfer Type            Interrupt
            Synch Type               None
            Usage Type               Data
            wMaxPacketSize     0x0008  1x 8 bytes
            bInterval               1
        Interface Descriptor:
        bLength                 9
        bDescriptorType         4
        bInterfaceNumber        1
        bAlternateSetting       0
        bNumEndpoints           2
        bInterfaceClass         3 Human Interface Device
        bInterfaceSubClass      1 Boot Interface Subclass
        bInterfaceProtocol      0 
        iInterface              0 
            HID Device Descriptor:
            bLength                 9
            bDescriptorType        33
            bcdHID               1.11
            bCountryCode            0 Not supported
            bNumDescriptors         1
            bDescriptorType        34 Report
            wDescriptorLength     124
            Report Descriptors: 
            ** UNAVAILABLE **
        Endpoint Descriptor:
            bLength                 7
            bDescriptorType         5
            bEndpointAddress     0x82  EP 2 IN
            bmAttributes            3
            Transfer Type            Interrupt
            Synch Type               None
            Usage Type               Data
            wMaxPacketSize     0x0040  1x 64 bytes
            bInterval               1
        Endpoint Descriptor:
            bLength                 7
            bDescriptorType         5
            bEndpointAddress     0x03  EP 3 OUT
            bmAttributes            3
            Transfer Type            Interrupt
            Synch Type               None
            Usage Type               Data
            wMaxPacketSize     0x0040  1x 64 bytes
            bInterval               1
    Device Status:     0x0002
    (Bus Powered)
    Remote Wakeup Enabled
