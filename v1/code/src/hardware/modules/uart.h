#pragma once

#include <stdint.h>

void uart_init(uint32_t baudrate);
void uart_send_char(char c);