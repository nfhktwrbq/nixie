#pragma once

#define SYSTEM_CORE_CLOCK_HZ   (72000000u)
#define APB1_CLOCK_HZ          (36000000u)
#define APB2_CLOCK_HZ          (72000000u)
#define LSE_CLOCK_HZ           (32768u)

void system_init(void);
