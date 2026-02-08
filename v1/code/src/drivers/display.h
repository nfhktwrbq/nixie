#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_WIDTH       (4u)
#define DISPLAY_NO_DOT      (false)
#define DISPLAY_NO_BLINK    (false)

void display_init_ll(void);
void display_digit_switch(void);
void display_char_set(char c, uint32_t pos, bool dot, bool blink);
void display_uint_set(uint32_t d);
void display_test_ll(void);
void display_dot_set(uint32_t pos, bool dot);
void display_second_set(bool en);
void display_turn_digits_off(void);
void display_turn_off(void);