#if STM32F103C6X == 1
#include "stm32f103x6.h"
#elif STM32F103C8X == 1
#include "stm32f103xb.h"
#else
#error "MK is not defiend"
#endif