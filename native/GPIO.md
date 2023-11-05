# GPIO module

The GPIO module provides a simple wrapper over the respective platforms
GPIO system. It exposes an object oriented interface. It gets initialized
via either

```
enum gpioError gpioOpen(struct gpio** lpOut, const char* lpFilename);
```

or

```
enum gpioError gpioOpenDefault(struct gpio** lpOut);
```

In the latter case the default GPIO port on the respective platform is used.

## Common operations

### Releasing / closing the GPIO subsystem

```
gpioObject->vtbl->release(gpioObject);
```

### Querying the number of I/O channels

```
unsigned long int channels;
enum gpioError e;

e = gpioObject->vtbl->getIOCount(gpioObject, &channels);
```

### Setting input or output direction

```
enum gpioError e;

e = gpioObject->vtbl->setModeInput(gpioObject, PINNUMBER);
e = gpioObject->vtbl->setModeOutput(gpioObject, PINNUMBER);
```

### Setting output state

```
e = gpioObject->vtbl->set(gpioObject, PINNUMBER, 1);
e = gpioObject->vtbl->set(gpioObject, PINNUMBER, 0);
```

### Pulsing an output port

```
# The delay is given in microseconds so to pulse for 1 millisecond:

e = gpioObject->vtbl->pulse(gpioObject, PINNUMBER, 1000);
```

### Reading an input port

```
int state;

e = gpioObject->vtbl->get(gpioObject, PINNUMBER, &state);
```

