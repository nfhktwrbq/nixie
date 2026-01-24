#pragma once

#include <stdint.h>

void system_init(void);

uint32_t cc_sys_clk_hz_get(void);
uint32_t cc_i2c_clk_hz_get(void);
uint32_t cc_rtc_clk_hz_get(void);
uint32_t cc_dbg_uart_clk_hz_get(void);
uint32_t cc_tim2_clk_hz_get(void);
