#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/gpio.h>

#include "./gpio.h"

struct gpioImpl {
	struct gpio		base;

	int			hHandle;
	unsigned long int	dwMaxCount;
};



static enum gpioError gpioImpl_Release(
	struct gpio* lpSelf
) {
	if(lpSelf == NULL) {
		return gpioE_InvalidParam;
	}

	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);

	if(lpThis->hHandle == -1) {
		return gpioE_InvalidParam;
	}
	close(lpThis->hHandle);
	free(lpThis);
	return gpioE_Ok;
}
static enum gpioError gpioImpl_GetIOCount(
	struct gpio* lpSelf,
	unsigned long int* lpCountOut
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }
	if(lpCountOut == NULL) { return gpioE_Ok; }

	(*lpCountOut) = 0;
	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);
	if(lpThis->hHandle < 0) { return gpioE_InvalidParam; }

	(*lpCountOut) = lpThis->dwMaxCount;
	return gpioE_Ok;
}
static enum gpioError gpioImpl_SetModeInput(
	struct gpio* lpSelf,
	unsigned long int pinNumber
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }

	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);

	if(pinNumber >= lpThis->dwMaxCount) {
		return gpioE_InvalidParam;
	}

	struct gpio_pin gpPin;
	gpPin.gp_pin = pinNumber;
	if(ioctl(lpThis->hHandle, GPIOGETCONFIG, &gpPin) < 0) {
		return gpioE_Failed;
	}

	gpPin.gp_flags = GPIO_PIN_INPUT;
	if(ioctl(lpThis->hHandle, GPIOSETCONFIG, &gpPin) < 0) {
		return gpioE_Failed;
	}

	return gpioE_Ok;
}
static enum gpioError gpioImpl_SetModeOutput(
	struct gpio* lpSelf,
	unsigned long int dwPinNumber
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }
	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);

	if(dwPinNumber >= lpThis->dwMaxCount) {
		return gpioE_InvalidParam;
	}
	struct gpio_pin gpPin;
	gpPin.gp_pin = dwPinNumber;
	if(ioctl(lpThis->hHandle, GPIOGETCONFIG, &gpPin) < 0) { return gpioE_Failed; }
	gpPin.gp_flags = GPIO_PIN_OUTPUT;
	if(ioctl(lpThis->hHandle, GPIOSETCONFIG, &gpPin) < 0) { return gpioE_Failed; }
	return gpioE_Ok;
}
static enum gpioError gpioImpl_Set(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	int state
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }
	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);
	if(pinNumber >= lpThis->dwMaxCount) { return gpioE_InvalidParam; }

	struct gpio_req rq;
	rq.gp_pin = pinNumber;
	rq.gp_value = (state == 0) ? 0 : 1;
	if(ioctl(lpThis->hHandle, GPIOSET, &rq) < 0) { return gpioE_Failed; }
	return gpioE_Ok;
}
static enum gpioError gpioImpl_Get(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	int* lpStateOut
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }
	if(lpStateOut == NULL) { return gpioE_Ok; }
	(*lpStateOut) = -1;

	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);
	if(pinNumber >= lpThis->dwMaxCount) { return gpioE_InvalidParam; }

	struct gpio_req rq;
	rq.gp_pin = pinNumber;
	if(ioctl(lpThis->hHandle, GPIOGET, &rq) < 0) { return gpioE_Failed; }
	(*lpStateOut) = rq.gp_value;
	return gpioE_Ok;
}
static enum gpioError gpioImpl_Pulse(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	unsigned long int dwMicrosecs
) {
	if(lpSelf == NULL) { return gpioE_InvalidParam; }

	struct gpioImpl* lpThis = (struct gpioImpl*)(lpSelf->lpReserved);
	if(pinNumber >= lpThis->dwMaxCount) { return gpioE_InvalidParam; }

	struct gpio_req rq;
	rq.gp_pin = pinNumber;

	if(ioctl(lpThis->hHandle, GPIOGET, &rq) < 0) { return gpioE_Failed; }

	rq.gp_value = (rq.gp_value == 0) ? 1 : 0;
	if(ioctl(lpThis->hHandle, GPIOSET, &rq) < 0) { return gpioE_Failed; }
	usleep(dwMicrosecs);
	rq.gp_value = (rq.gp_value == 0) ? 1 : 0;
	if(ioctl(lpThis->hHandle, GPIOSET, &rq) < 0) { return gpioE_Failed; }

	return gpioE_Ok;
}



struct gpioVtbl gpioVtblDefault = {
	&gpioImpl_Release,

	&gpioImpl_GetIOCount,
	&gpioImpl_SetModeInput,
	&gpioImpl_SetModeOutput,

	&gpioImpl_Set,
	&gpioImpl_Get,
	&gpioImpl_Pulse
};
#ifdef __cplusplus
	extern "C" {
#endif

enum gpioError gpioOpen(struct gpio** lpOut, const char* lpFilename) {
	struct gpioImpl* gpNew;

	if(lpOut == NULL) {
		return gpioE_InvalidParam;
	}
	(*lpOut) = (struct gpio*)NULL;

	if(lpFilename == NULL) {
		lpFilename = "/dev/gpioc0";
	}

	gpNew = (struct gpioImpl*)malloc(sizeof(struct gpioImpl));
	if(gpNew == (void*)NULL) {
		return gpioE_OutOfMemory;
	}

	gpNew->base.lpReserved = (void*)gpNew;
	gpNew->base.vtbl = &gpioVtblDefault;

	gpNew->hHandle = -1;
	gpNew->dwMaxCount = 0;

	/* Open device, determine maximum I/O count */
	gpNew->hHandle = open(lpFilename, O_RDONLY);
	if(gpNew->hHandle < 0) {
		free(gpNew);
		return gpioE_Failed;
	}

	int maxPins;
	if(ioctl(gpNew->hHandle, GPIOMAXPIN, &maxPins) < 0) {
		close(gpNew->hHandle);
		free(gpNew);
		return gpioE_Failed;
	}
	gpNew->dwMaxCount = (unsigned long int)maxPins;

	(*lpOut) = &(gpNew->base);
	return gpioE_Ok;
}

enum gpioError gpioOpenDefault(struct gpio** lpOut) {
	return gpioOpen(lpOut, (void*)NULL);
}

#ifdef __cplusplus
	} /* extern "C" */
#endif
