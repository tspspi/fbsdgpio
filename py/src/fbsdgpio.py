import fcntl
import struct
import os

from enum import Enum

from labdevices.gpio import GpioDirection, GpioDrive, GpioPulls, GPIO

class FbsdGPIO(GPIO):
    def __init__(
            self,
            gpiodev = "/dev/gpioc0",
            validatePortConfig = False
    ):
        self._GPIOGET       = 3221767939
        self._GPIOSET       = 2148026116
        self._GPIOGETCONFIG = 3226224385
        self._GPIOSETCONFIG = 2152482562
        self._GPIOMAXPIN    = 1074022144

        self._handle = os.open(gpiodev, os.O_RDONLY)
        self._maxio = struct.unpack('l', fcntl.ioctl(self._handle, self._GPIOMAXPIN, struct.pack('l', 0)))[0]

        self._validatePortConfig = validatePortConfig

        super().__init__(self._maxio)

    def __del__(self):
        # (Deferred) closing of the handle
        try:
            os.close(self._handle)
        except:
            pass

    def __enter__(self):
        return self
    def __exit__(self, type, value, tb):
        return

    def _getConfig(self, pin):
        if (pin >= self._maxio) or (pin < 0):
            raise ValueError("Pin number {pin} not supported, required to be in range 0 to {self._maxio-1}")
        #
        #    The structure contains:
        #        uint32_t    gpPin
        #        char        gp_name[64];
        #        uint32_t    gp_caps;
        #        uint32_t    gp_flags;
        #
        res = struct.unpack("l64sll", fcntl.ioctl(self._handle, self._GPIOGETCONFIG, struct.pack("l64sll", pin, bytes([0]), 0 , 0)))

        pinnum = res[0]
        pinname = res[1].decode("utf-8").strip('\0')
        pindirs = []
        pindrives = []
        pinpulls = []
        invertIn = False
        invertOut = False
        hwPulse = False

        if (res[2] & GpioDirection.INPUT.value) != 0:
            pindirs.append(GpioDirection.INPUT)
        if (res[2] & GpioDirection.OUTPUT.value) != 0:
            pindirs.append(GpioDirection.OUTPUT)
        if (res[2] & GpioDrive.OPENDRAIN.value) != 0:
            pindrives.append(GpioDrive.OPENDRAIN)
        if (res[2] & GpioDrive.TRISTATE.value) != 0:
            pindrives.append(GpioDrive.TRISTATE)
        if (res[2] & GpioPulls.PULLUP.value) != 0:
            pinpulls.append(GpioPulls.PULLUP)
        if (res[2] & GpioPulls.PULLDOWN.value) != 0:
            pinpulls.append(GpioPulls.PULLDOWN)
        if (res[2] & 0x80) != 0:
            invertIn = True
        if (res[2] & 0x100) != 0:
            invertOut = True
        if (res[2] & 0x200) != 0:
            hwPulse = True

        caps = {
            'direction' : pindirs,
            'drives' : pindrives,
            'pull' : pinpulls,
            'invertInput' : invertIn,
            'invertOutput' : invertOut,
            'hardwarePulsate' : hwPulse
        }

        pindirs = []
        pindrives = []
        pinpulls = []
        invertIn = False
        invertOut = False
        hwPulse = False

        if (res[3] & GpioDirection.INPUT.value) != 0:
            pindirs.append(GpioDirection.INPUT)
        if (res[3] & GpioDirection.OUTPUT.value) != 0:
            pindirs.append(GpioDirection.OUTPUT)
        if (res[3] & GpioDrive.OPENDRAIN.value) != 0:
            pindrives.append(GpioDrive.OPENDRAIN)
        if (res[3] & GpioDrive.TRISTATE.value) != 0:
            pindrives.append(GpioDrive.TRISTATE)
        if (res[3] & GpioPulls.PULLUP.value) != 0:
            pinpulls.append(GpioPulls.PULLUP)
        if (res[3] & GpioPulls.PULLDOWN.value) != 0:
            pinpulls.append(GpioPulls.PULLDOWN)
        if (res[3] & 0x80) != 0:
            invertIn = True
        if (res[3] & 0x100) != 0:
            invertOut = True
        if (res[3] & 0x200) != 0:
            hwPulse = True

        pindirs = pindirs[0]
        if len(pindrives) > 0:
            pindrives = pindrives[0]
        else:
            pindrives = None
        if len(pinpulls) > 0:
            pinpulls = pinpulls[0]
        else:
            pinpulls = None

        current = {
            'direction' : pindirs,
            'pindrive' : pindrives,
            'pull' : pinpulls,
            'invertInput' : invertIn,
            'invertOutput' : invertOut,
            'hardwarePulsate' : hwPulse,
        }

        return {
            'io' : pinnum,
            'name' : pinname,
            'capabilities' : caps,
            'configuration' : current,

            'raw' : {
                'caps' : res[2],
                'flags' : res[3]
            }
        }

    def _setConfig(
        self,
        pin,
        name = None,
        direction = None,
        pull = None,
        drive = None,
        invertInput = False,
        invertOutput = False,
        hardwarePulsate = False
    ):
        currentCfg = self.getConfig(pin)

        if name is None:
            name = currentCfg['name']
        else:
            if len(name) > 63:
                raise ValueError("New pin name too long")

        caps = currentCfg['raw']['caps']
        flags = currentCfg['raw']['flags']

        if direction is not None:
            if direction not in currentCfg['capabilities']['direction']:
                raise ValueError(f"Pin {pin} does not support direction {direction} (supported: {currentCfg['capabilities']['direction']})")
            flags = flags & (~GpioDirection.MASK.value)
            flags = flags | direction.value
        if pull is not None:
            if pull not in currentCfg['capabilities']['pull']:
                raise ValueError(f"Pullup/Pulldown configuration of pin {pin} not supporting {pull} (supported: {currentCfg['capabilities']['pull']})")
            flags = flags & (~GpioPulls.MASK.value)
            flags = flags | pull.value
        if drive is not None:
            if drive not in currentCfg['capabilities']['pindrive']:
                raise ValueError(f"Drive configuration {drive} of pin {pin} not supported (supported: {currentCfg['capabilities']['pull']})")
            flags = flags & (~GpioDrive.MASK.value)
            flags = flags | drive.value
        if invertInput:
            if not currentCfg['capabilities']['invertInput']:
                raise ValueError(f"Pin {pin} does not support input inversion")
            flags = flags | 0x80
        if invertOutput:
            if not currentCfg['capabilities']['invertOutput']:
                raise ValueError(f"Pin {pin} does not support output inversion")
            flags = flags | 0x100

        res = fcntl.ioctl(self._handle, self._GPIOSETCONFIG, struct.pack("l64sll", pin, str(name).encode("utf-8"), caps, flags))
        return True

    def _set(
        self,
        pin,
        status
    ):
        if (pin < 0) or (pin >= self._maxio):
            raise ValueError(f"I/O port number {pin} out of range from 0 to {self._maxio-1}")

        if self._validatePortConfig:
            cfg = self.getConfig(pin)
            if cfg['configuration']['direction'] != GpioDirection.OUTPUT:
                raise ValueError(f"Port {pin} is not configured as output, cannot set output")

        if status:
            status = 1

        fcntl.ioctl(self._handle, self._GPIOSET, struct.pack("ll", pin, status))
        return True

    def _get(
        self,
        pin
    ):
        if (pin < 0) or (pin >= self._maxio):
            raise ValueError(f"I/O port number {pin} out of range from 0 to {self._maxio-1}")
        if self._validatePortConfig:
            cfg = self.getConfig(pin)
            if cfg['configuration']['direction'] != GpioDirection.INPUT:
                raise ValueError(f"Port {pin} is not configured as input, cannot read input value")

        _, status = struct.unpack("ll", fcntl.ioctl(self._handle, self._GPIOGET, struct.pack("ll", pin, 0)))

        return status != 0

    def _pulse(
        self,
        pin,
        microseconds,
        state = None
    ):
        if (pin < 0) or (pin >= self._maxio):
            raise ValueError(f"I/O port number {pin} is out of range from 0 to {self._maxio-1}")
        if self._validatePortConfig:
            cfg = self.getConfig(pin)
            if cfg['configuration']['direction'] != GpioDirection.OUTPUT:
                raise ValueError(f"Port {pin} is not configured as output, cannot pulse output")

        oldstate = None

        if state is None:
            _, oldstate = struct.unpack("11", fcntl.ioctl(self._handle, self._GPIOGET, struct.pack("ll", pin, 0)))
            oldstate = (status != 0)
            state = (status == 0)
        else:
            if state:
                state = True
                oldstate = False
            else:
                state = False
                oldstate = True

        self.set(pin, state)
        sleep(float(microseconds) / 1e6)
        self.set(pin, oldstate)

        return True

