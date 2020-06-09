class Light2Error(Exception):
	pass

class Dpi:
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
		"""Converts the DPI object to 3 bytes structure

		Returns:
			bytes: DPI values in 3 bytes structure
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

		DPI for x- and y axis is stored in steps of 50. The regular value must be divided by 50.
		The device supports regular values between 50 and 16000.
		As x and y are limited to one byte, the rest is stored in an offset byte.
		The first 4 bits are for x and the last 4 for y.
		Sample
		0x0 0x1 0xf0 0x2c  | x = 240  Ofx = 0 | ((Ofx * 256) + y) * 50 => 12000 DPI 
		Ofx Ofy  x    y    | y = 44   Ofy = 1 | ((Ofy * 256) + x) * 50 => 15000 DPI

		Args:
			b (bytes): DPI values in 3 bytes

		Returns:
			Dpi: DPI object in human readable units
		"""
	
		if len(b) != 3:
			raise Light2Error('Wrong byte size, 3 bytes are expected.')

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
	def __init__(self):
		"""Creates a DpiSettings object
		"""
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
		"""Converts the DpiSettings object to 22 bytes structure

		byte 1 => state of the seven DPI steps decode in bits
		sample - bit mask if all steps are enabled
		 0    1     1     1     1     1     1     1
		 -   DPI7  DPI6  DPI5  DPI4  DPI3  DPI2  DPI1

		bytes 2-22 => DPI values for each step

		Returns:
			bytes: DpiSettings in 22 bytes structure
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

		b = bytes([steps])
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
		"""Creates an DpiSettings object from 22 bytes

		byte 1 => state of the seven DPI steps decode in bits
		sample - bit mask if all steps are enabled
		 0    1     1     1     1     1     1     1
		 -   DPI7  DPI6  DPI5  DPI4  DPI3  DPI2  DPI1

		bytes 2-22 => DPI values for each step

		Args:
			b (bytes): dpi settings in 22 bytes

		Returns:
			DpiSettings: DpiSettings object
		"""

		if len(b) != 22:
			raise Light2Error('Wrong byte size, 22 bytes are expected.')

		ds = DpiSettings()
		ds.dpi1_on = bool(b[0] & 1)
		ds.dpi2_on = bool((b[0] >> 1) & 1)
		ds.dpi3_on = bool((b[0] >> 2) & 1)
		ds.dpi4_on = bool((b[0] >> 3) & 1)
		ds.dpi5_on = bool((b[0] >> 4) & 1)
		ds.dpi6_on = bool((b[0] >> 5) & 1)
		ds.dpi7_on = bool((b[0] >> 6) & 1)

		ds.dpi1_value = Dpi.fromBytes(b[1:4])
		ds.dpi2_value = Dpi.fromBytes(b[4:7])
		ds.dpi3_value = Dpi.fromBytes(b[7:10])
		ds.dpi4_value = Dpi.fromBytes(b[10:13])
		ds.dpi5_value = Dpi.fromBytes(b[13:16])
		ds.dpi6_value = Dpi.fromBytes(b[16:19])
		ds.dpi7_value = Dpi.fromBytes(b[19:22])

		return ds;

class IlluminationSettings:
	def __init__(self):
	  self.led_effect = 0
	  self.led_frequency = 1
	  self.led_brightness = 10
	  self.unknownByte41 = 1
	  self.profile_id = 0
	  self.color1 = (0,0,0)
	  self.color2 = (0,0,0)
	  self.color3 = (0,0,0)
	  self.color4 = (0,0,0)
	  self.color5 = (0,0,0)
	  self.color6 = (0,0,0)
	  self.color7 = (0,0,0)

	def __eq__(self,other):
		"""Comparison with other object on the basis of their attributes

		Args:
			other (DpiSettings): other DpiSettings object

		Returns:
			bool: true if all attribute values are equal
		"""
		if not isinstance(other, IlluminationSettings):
			return NotImplemented
		else:
			for attr1, attr2 in zip(self.__dict__, other.__dict__):
				if attr1 != attr2:
					return False
				if getattr(self, attr1) != getattr(other, attr2):
					return False
			
			return True

	def toBytes(self) -> bytes:
		"""Converts the IlluminationSettings object to 26 bytes structure

		byte 1 => state of the seven DPI steps decode in bits
		sample - bit mask if all steps are enabled
		 0    1     1     1     1     1     1     1
		 -   DPI7  DPI6  DPI5  DPI4  DPI3  DPI2  DPI1

		bytes 2-22 => DPI values for each step

		Returns:
			bytes: DpiSettings in 22 bytes structure
		"""
		b = bytes([self.led_effect,
				   self.led_frequency,
				   self.led_brightness,
				   self.unknownByte41,
				   self.profile_id])
		b += bytes([self.color1[0], self.color1[1], self.color1[2]])
		b += bytes([self.color2[0], self.color2[1], self.color2[2]])
		b += bytes([self.color3[0], self.color3[1], self.color3[2]])
		b += bytes([self.color4[0], self.color4[1], self.color4[2]])
		b += bytes([self.color5[0], self.color5[1], self.color5[2]])
		b += bytes([self.color6[0], self.color6[1], self.color6[2]])
		b += bytes([self.color7[0], self.color7[1], self.color7[2]])

		return b

	@classmethod
	def fromBytes(self, b: bytes):
		"""Creates an IlluminationSettings object from 26 bytes

		Args:
			b (bytes): illumination settings in 26 bytes

		Returns:
			IlluminationSettings: IlluminationSettings object
		"""

		if len(b) != 26:
			raise Light2Error('Wrong byte size, 26 bytes are expected.')

		ils = IlluminationSettings()
		ils.led_effect = b[0]
		ils.led_frequency = b[1]
		ils.led_brightness = b[2]
		ils.unknownByte41 = b[3]
		ils.profile_id = b[4]
		ils.color1 = (b[5], b[6], b[7])
		ils.color2 = (b[8], b[9], b[10])
		ils.color3 = (b[11], b[12], b[13])
		ils.color4 = (b[14], b[15], b[16])
		ils.color5 = (b[17], b[18], b[19])
		ils.color6 = (b[20], b[21], b[22])
		ils.color7 = (b[23], b[24], b[25])

		return ils