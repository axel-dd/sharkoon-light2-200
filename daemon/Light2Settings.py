from enum import IntEnum

class Light2Error(Exception):
	pass

class Dpi:
	BYTE_SIZE = 3
	
	"""DPI values for x-axis and y-axis is stored in steps of 50. The regular value must be divided by 50.
	   The Sharkoon Light2 200 mouse supports regular values between 50 and 16000.
	"""
	def __init__(self, x, y):
		"""Creates a Dpi object in human readable units

		Args:
			x (int): DPI value of x-axis in human readable units (value range between 50 and 16000 in steps of 50)
			y (int): DPI value of y-axis in human readable units (value range between 50 and 16000 in steps of 50)
		"""
		if x < 50 or x > 16000:
			raise Light2Error('Invalid value range for x')
		if y < 50 or y > 16000:
			raise Light2Error('Invalid value range for y')
		if x % 50 != 0:
			raise Light2Error('Invalid values for x')
		if y % 50 != 0:
			raise Light2Error('Invalid values for y')

		self.x = x
		self.y = y

	def __eq__(self,other):
		"""Comparison with other object on the basis of their attributes

		Args:
			other (Dpi): other DPI object

		Returns:
			bool: true if all attribute values are equal
		"""
		if not isinstance(other, Dpi):
			return NotImplemented
		else:
			for attr1, attr2 in zip(self.__dict__, other.__dict__):
				if attr1 != attr2:
					return False
				if getattr(self, attr1) != getattr(other, attr2):
					return False
			
			return True

	def toBytes(self) -> bytes:
		"""Converts the DPI object to Dpi.BYTE_SIZE bytes structure

		Returns:
			bytes: DPI values in Dpi.BYTE_SIZE bytes structure
		"""
		x = int(self.x / 50)
		y = int(self.y / 50)

		#  0x0 [0x1] 0x0 0x0
    	# [0x1] 0x0
		offset = (x >> 4) & 0xF0

		# 0x0 [0x1] 0x0 0x0
		# 0x0 [0x1]
		offset |= (y >> 8) & 0x0F

		x = x & 0xFF
		y = y & 0xFF

		return bytes([offset, x, y])


	@classmethod
	def fromBytes(self, b: bytes):
		"""Creates an DPI object from bytes

		Args:
			b (bytes): DPI values in Dpi.BYTE_SIZE bytes

		Returns:
			Dpi: DPI object in human readable units
		"""
	
		if len(b) != Dpi.BYTE_SIZE:
			raise Light2Error(f'Wrong byte size, {Dpi.BYTE_SIZE} bytes are expected.')

		#           [0x1] 0x0
    	# 0x0 [0x1]  0x0  0x0
		x = (b[0] << 4) & 0x0F00
		x |= b[1]
		x *= 50

		#           0x0 [0x1]
    	# 0x0 [0x1] 0x0  0x0
		y = (b[0] << 8) & 0x0F00
		y |= b[2]
		y *= 50

		return Dpi(x, y)


class DpiSettings:
	BYTE_SIZE = 23

	def __init__(self):
		"""Creates a DpiSettings object
		"""
		self.currentDpiStep = 1

		self.dpi1_on = True
		self.dpi2_on = True
		self.dpi3_on = True
		self.dpi4_on = True
		self.dpi5_on = True
		self.dpi6_on = True
		self.dpi7_on = True

		self.dpi1_value = Dpi(400, 400)
		self.dpi2_value = Dpi(800, 800)
		self.dpi3_value = Dpi(1200, 1200)
		self.dpi4_value = Dpi(2400, 2400)
		self.dpi5_value = Dpi(3200, 3200)
		self.dpi6_value = Dpi(6400, 6400)
		self.dpi7_value = Dpi(16000, 16000)

	def __eq__(self,other):
		"""Comparison with other object on the basis of their attributes

		Args:
			other (DpiSettings): other DpiSettings object

		Returns:
			bool: true if all attribute values are equal
		"""
		if not isinstance(other, DpiSettings):
			return NotImplemented
		else:
			for attr1, attr2 in zip(self.__dict__, other.__dict__):
				if attr1 != attr2:
					return False
				if getattr(self, attr1) != getattr(other, attr2):
					return False
			
			return True

	def toBytes(self) -> bytes:
		"""Converts the DpiSettings object to DpiSettings.BYTE_SIZE bytes structure

		Returns:
			bytes: DpiSettings in DpiSettings.BYTE_SIZE bytes structure
		"""
		steps = 0
		if self.dpi1_on:
			steps = 1
		if self.dpi2_on:
			steps |= 1 << 1
		if self.dpi3_on:
			steps |= 1 << 2
		if self.dpi4_on:
			steps |= 1 << 3
		if self.dpi5_on:
			steps |= 1 << 4
		if self.dpi6_on:
			steps |= 1 << 5
		if self.dpi7_on:
			steps |= 1 << 6

		b = bytes([self.currentDpiStep])
		b += bytes([steps])
		b += self.dpi1_value.toBytes()
		b += self.dpi2_value.toBytes()
		b += self.dpi3_value.toBytes()
		b += self.dpi4_value.toBytes()
		b += self.dpi5_value.toBytes()
		b += self.dpi6_value.toBytes()
		b += self.dpi7_value.toBytes()

		return b

	@classmethod
	def fromBytes(self, b: bytes):
		"""Creates an DpiSettings object from DpiSettings.BYTE_SIZE bytes

		Args:
			b (bytes): dpi settings in DpiSettings.BYTE_SIZE bytes

		Returns:
			DpiSettings: DpiSettings object
		"""

		if len(b) != DpiSettings.BYTE_SIZE:
			raise Light2Error(f'Wrong byte size, {DpiSettings.BYTE_SIZE} bytes are expected.')

		ds = DpiSettings()
		ds.currentDpiStep = b[0]

		ds.dpi1_on = bool(b[1] & 1)
		ds.dpi2_on = bool((b[1] >> 1) & 1)
		ds.dpi3_on = bool((b[1] >> 2) & 1)
		ds.dpi4_on = bool((b[1] >> 3) & 1)
		ds.dpi5_on = bool((b[1] >> 4) & 1)
		ds.dpi6_on = bool((b[1] >> 5) & 1)
		ds.dpi7_on = bool((b[1] >> 6) & 1)

		ds.dpi1_value = Dpi.fromBytes(b[2:5])
		ds.dpi2_value = Dpi.fromBytes(b[5:8])
		ds.dpi3_value = Dpi.fromBytes(b[8:11])
		ds.dpi4_value = Dpi.fromBytes(b[11:14])
		ds.dpi5_value = Dpi.fromBytes(b[14:17])
		ds.dpi6_value = Dpi.fromBytes(b[17:20])
		ds.dpi7_value = Dpi.fromBytes(b[20:23])

		return ds

class LedEffect(IntEnum):
	PULSATING_RGB_CYCLE = 0
	PULSATING = 1
	PERMANENT = 2
	COLOR_CHANGE = 3
	SINGLE_COLOR_MARQUEE = 4
	MULTI_COLOR_MARQUEE = 5
	RIPPLE_EFFECT = 6
	TRIGGER = 7
	HEARTBEAT = 8
	LED_OFF = 9

class SettingsMessage:
	BYTE_SIZE = 64

	def __init__(self):
		self.version = b'\x04' # byte 1
		self.message_type = b'\xa0\x01' # byte 2-3
		self.settings_command = b'\x02' # byte 4
		self.bytes5_7 = b'\x01\x02\xa5' # byte 5-7
		self.dpi_settings = DpiSettings() # byte 8-30
		self.bytes31_33 = b'\x00\x00\x00' # byte 31-33
		self.lift_off_distance = 2 # byte 34
		self.bytes35_37 = b'\x02\x00\xa5' # byte 35-37
		self.led_effect = LedEffect.PULSATING_RGB_CYCLE # byte 38
		self.led_frequency = 1 # byte 39
		self.led_brightness = 10 # byte 40
		self.profile_id = 1 # byte 41
		self.byte42 = b'\x00' # byte 42
		self.colorRgb1 = (0,0,0) # byte 43-45
		self.colorRgb2 = (0,0,0) # byte 46-48
		self.colorRgb3 = (0,0,0) # byte 49-51
		self.colorRgb4 = (0,0,0) # byte 52-54
		self.colorRgb5 = (0,0,0) # byte 55-57
		self.colorRgb6 = (0,0,0) # byte 58-60
		self.colorRgb7 = (0,0,0) # byte 61-63
		self.byte64 = b'\x00' # byte 64

	def __eq__(self,other):
		"""Comparison with other object on the basis of their attributes

		Args:
			other (DpiSettings): other DpiSettings object

		Returns:
			bool: true if all attribute values are equal
		"""
		if not isinstance(other, SettingsMessage):
			return NotImplemented
		else:
			for attr1, attr2 in zip(self.__dict__, other.__dict__):
				if attr1 != attr2:
					return False
				if getattr(self, attr1) != getattr(other, attr2):
					return False
			
			return True

	def toBytes(self) -> bytes:
		"""Converts the SettingsMessage object to SettingsMessage.BYTE_SIZE bytes structure

		Returns:
			bytes: SettingsMessage in SettingsMessage.BYTE_SIZE bytes structure
		"""
		b = self.version
		b += self.message_type
		b += self.settings_command
		b += self.bytes5_7
		b += self.dpi_settings.toBytes()
		b += self.bytes31_33
		b += bytes([self.lift_off_distance])
		b += self.bytes35_37
		b += bytes([self.led_effect])
		b += bytes([self.led_frequency])
		b += bytes([self.led_brightness])
		b += bytes([self.profile_id])
		b += self.byte42
		b += bytes(self.colorRgb1[0:3])
		b += bytes(self.colorRgb2[0:3])
		b += bytes(self.colorRgb3[0:3])
		b += bytes(self.colorRgb4[0:3])
		b += bytes(self.colorRgb5[0:3])
		b += bytes(self.colorRgb6[0:3])
		b += bytes(self.colorRgb7[0:3])
		b += self.byte64

		return b

	@classmethod
	def fromBytes(self, b: bytes):
		"""Creates an SettingsMessage object from SettingsMessage.BYTE_SIZE bytes

		Args:
			b (bytes): dpi settings in SettingsMessage.BYTE_SIZE bytes

		Returns:
			SettingsMessage: SettingsMessage object
		"""

		if len(b) != SettingsMessage.BYTE_SIZE:
			raise Light2Error(f'Wrong byte size, {SettingsMessage.BYTE_SIZE} bytes are expected.')

		msg = SettingsMessage()
		msg.version = bytes([b[0]])
		msg.message_type = b[1:3]
		msg.settings_command = bytes([b[3]])
		msg.bytes5_7 = b[4:7]
		msg.dpi_settings = DpiSettings.fromBytes(b[7:30])
		msg.bytes31_33 = b[30:33]
		msg.lift_off_distance = int(b[33])
		msg.bytes35_37 = b[34:37]
		msg.led_effect = LedEffect(b[37])
		msg.led_frequency = int(b[38])
		msg.led_brightness = int(b[39])
		msg.profile_id = int(b[40])
		msg.byte42 = bytes([b[41]])
		msg.colorRgb1 = (int(b[42]), int(b[43]), int(b[44]))
		msg.colorRgb2 = (int(b[45]), int(b[46]), int(b[47]))
		msg.colorRgb3 = (int(b[48]), int(b[49]), int(b[50]))
		msg.colorRgb4 = (int(b[51]), int(b[52]), int(b[53]))
		msg.colorRgb5 = (int(b[54]), int(b[55]), int(b[56]))
		msg.colorRgb6 = (int(b[57]), int(b[58]), int(b[59]))
		msg.colorRgb7 = (int(b[60]), int(b[61]), int(b[62]))
		msg.byte64 = bytes([b[63]])

		return msg