import Light2Settings
import binascii

def TestDpiConversation(byteData: bytes, x: int, y: int):
    dpi = Light2Settings.Dpi.fromBytes(byteData)
    if dpi.x != x:
        raise Exception()
    if dpi.y != y:
        raise Exception()
    if byteData != dpi.toBytes():
        raise Exception()

def TestWithInvalidValues(x: int, y: int):
    try:
        dpi = Light2Settings.Dpi(x, y)
    except Light2Settings.Light2Error:
        pass
    else:
        raise Exception()

def TestDpiSettingsByteSize():
    if len(Light2Settings.DpiSettings().toBytes()) != 22:
        raise Exception()

def TestDpiSettingsConversation():
    # compare with default byte data
    b = binascii.unhexlify('7f000808001010001818003030004040008080114040')
    s = Light2Settings.DpiSettings.fromBytes(b)
    if s != Light2Settings.DpiSettings():
        raise Exception()


TestDpiConversation(bytes([0x11, 0x40, 0x40]), 16000, 16000)
TestDpiConversation(bytes([0x00, 0x10, 0x10]), 800, 800)
TestDpiConversation(bytes([0x01, 0xf0, 0x2c]), 12000, 15000)
TestDpiConversation(bytes([0x10, 0x1d, 0x88]), 14250, 6800)

TestWithInvalidValues(0, 50)
TestWithInvalidValues(50, 77)
TestWithInvalidValues(-50, 50)
TestWithInvalidValues(50, -100)
TestWithInvalidValues(16001, 50)
TestWithInvalidValues(50, 16050)

TestDpiSettingsByteSize()
TestDpiSettingsConversation()