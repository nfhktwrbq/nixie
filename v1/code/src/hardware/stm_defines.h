#pragma once


#ifdef STM32F103C6X 
#include "stm32f103x6.h"
#elif defined(STM32F103C8X)
#include "stm32f103xb.h"
#else
#error "MK is not defiend"
#endif