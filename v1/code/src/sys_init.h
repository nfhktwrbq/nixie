#pragma once

#define SYSTEM_CORE_CLOCK_HZ   (72000000u)
#define APB1_CLOCK_HZ          (36000000u)

void SystemInit(void);
void uart_send_char(char c);
void systick_handler(void);