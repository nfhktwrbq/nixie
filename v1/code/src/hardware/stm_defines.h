#pragma once


#ifdef STM32F103C6X 
#include "stm32f103x6.h"

#define STM_DEFS_RTOS_HEAP_SIZE       ( 7 * 1024 )

#elif defined(STM32F103C8X)

#include "stm32f103xb.h"

#define STM_DEFS_RTOS_HEAP_SIZE        ( 17 * 1024 )

#else
#error "MK is not defiend"
#endif


#include "core/core_cm3.h"