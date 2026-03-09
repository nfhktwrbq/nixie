#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DISPLAY_WIDTH       (4u)
#define DISPLAY_NO_DOT      (false)
#define DISPLAY_NO_BLINK    (false)

void display_init_ll(void);
void display_digit_switch(void);
void display_char_set(char c, uint32_t pos);
void display_char_set_ex(char c, uint32_t pos, bool dot, bool blink);
void display_blink_set(uint32_t pos, bool blink);
void display_blink_msk_set(uint32_t msk);
void display_uint_set(uint32_t d);
void display_test_ll(void);
void display_dot_set(uint32_t pos, bool dot);
void display_dot_msk_set(uint32_t msk);
void display_second_set(bool en);
void display_turn_digits_off(void);
void display_turn_off(void);
void display_print(const char * str);
void display_clear(void);