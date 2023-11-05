from labdevices.gpio import GpioDirection
from fbsdgpio import FbsdGPIO

from time import sleep

with FbsdGPIO() as gpio:
	print(f"Pin count: {gpio.getIOCount()}")
	print(f"Pin configuration of pin 13: {gpio.getConfig(13)}")
	print(f"Setting configuration to input on pin 13: {gpio.setConfig(13, direction=GpioDirection.INPUT)}")
	sleep(10)
	print(f"Setting configuration to output on pin 13: {gpio.setConfig(13, direction=GpioDirection.OUTPUT)}")
