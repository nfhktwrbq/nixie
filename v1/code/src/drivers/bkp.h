#pragma once

#include <stdint.h>

void bkp_data_write(uint8_t registerNumber, uint16_t data);
uint16_t bkp_data_read(uint8_t registerNumber);
