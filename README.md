# GPIO wrapper for FreeBSD

This is a simple collection of GPIO wrappers for various languages
for the FreeBSD gpio device. Currently it contains:

* An native ANSI C wrapper in ```native```
* A Python wrapper based on the ```ioctl``` interface of the ```fcntl```
  module. This is an implementation of the ```GPIO``` class
  of the [labdevices GPIO base class](https://github.com/tspspi/pylabdevs/blob/master/src/labdevices/gpio.py)

Those wrappers have been used to access the GPIO pins on platforms
like the RaspberryPi.
