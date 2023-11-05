# FreeBSD GPIO ioctl wrapper for Python

This is a simple wrapper around the FreeBSD ```ioctl``` calles for the
GPIO device. It allows very simple access to configuration and I/O facilities
of the GPIO pins.

## Installation

```
pip install pyfbsdgpio-tspspi
```

## Basic usage

For now this section only summarizes the most basic usage of the module (it
also supports setting pullup and pulldown resistors and selecting different
drive characteristics if the GPIO chipset supports it).

### Determining pin count of the GPIO module

One can determine the number of I/O pins available on the GPIO device:

```
from labdevices.gpio import GpioDirection, GpioDrive, GpioPull
from fbsdgpio import FbsdGPIO

gpio = FbsdGPIO()
print(f"We have {gpio.getIOCount()} I/O ports")
```

### Fetching the current configuration of an I/O port

```
from labdevices.gpio import GpioDirection, GpioDrive, GpioPull
from fbsdgpio import FbsdGPIO

gpio = FbsdGPIO()
print(f"Current configuration of pin 13: {gpio.getConfig(13)}")
```

### Setting pin as input or output

```
from labdevices.gpio import GpioDirection, GpioDrive, GpioPull
from fbsdgpio import FbsdGPIO

with FbsdGPIO() as gpio:
    gpio.setConfig(13, direction=GpioDirection.Input)
```

```
with FbsdGPIO() as gpio:
    gpio.setConfig(13, direction=GpioDirection.Output)
```

### Pulse output

```
# Pulse for 100 microseconds

with FbsdGPIO() as gpio:
    gpio.pulse(13, 100)
```

Also specifying the pulsed (not current) state so the module
does not have to query it:

```
with FbsdGPIO() as gpio:
    gpio.pulse(13, 100, False)
```
