#ifndef __is_included__ffcc55d5_77a0_11ee_a349_b499badf00a1
#define __is_included__ffcc55d5_77a0_11ee_a349_b499badf00a1

#ifdef __cplusplus
	extern "C" {
#endif

enum gpioError {
	gpioE_Ok		= 0,

	gpioE_InvalidParam,
	gpioE_OutOfMemory,
	gpioE_Failed,
};

struct gpio;
struct gpioVtbl;

typedef enum gpioError (*lpfnGpio_Release)(
	struct gpio* lpSelf
);
typedef enum gpioError (*lpfnGpio_GetIOCount)(
	struct gpio* lpSelf,
	unsigned long int* lpCountOut
);
typedef enum gpioError (*lpfnGpio_SetModeInput)(
	struct gpio* lpSelf,
	unsigned long int pinNumber
);
typedef enum gpioError (*lpfnGpio_SetModeOutput)(
	struct gpio* lpSelf,
	unsigned long int pinNumber
);
typedef enum gpioError (*lpfnGpio_Set)(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	int state
);
typedef enum gpioError (*lpfnGpio_Get)(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	int* lpStateOut
);
typedef enum gpioError (*lpfnGpio_Pulse)(
	struct gpio* lpSelf,
	unsigned long int pinNumber,
	unsigned long int dwMicroseconds
);

struct gpioVtbl {
	lpfnGpio_Release	release;

	lpfnGpio_GetIOCount	getIOCount;
	lpfnGpio_SetModeInput	setModeInput;
	lpfnGpio_SetModeOutput	setModeOutput;

	lpfnGpio_Set		set;
	lpfnGpio_Get		get;
	lpfnGpio_Pulse		pulse;
};

struct gpio {
	struct gpioVtbl*	vtbl;
	void*			lpReserved;
};

/* Constructor methods */

enum gpioError gpioOpen(struct gpio** lpOut, const char* lpFilename);
enum gpioError gpioOpenDefault(struct gpio** lpOut);

#ifdef __cplusplus
	} /* extern "C" */
#endif 

#endif
