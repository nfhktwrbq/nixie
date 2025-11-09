#pragma once

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_NTOA_BUFFER_SIZE
#define PRINTF_NTOA_BUFFER_SIZE    32U
#endif

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
// default: 32 byte
#ifndef PRINTF_FTOA_BUFFER_SIZE
#define PRINTF_FTOA_BUFFER_SIZE    0U
#endif

// support for the floating point type (%f)
// default: activated
#define PRINTF_DISABLE_SUPPORT_FLOAT
// #ifndef PRINTF_DISABLE_SUPPORT_FLOAT
// #define PRINTF_SUPPORT_FLOAT
// #endif

// support for exponential floating point notation (%e/%g)
// default: activated
#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
// #ifndef PRINTF_DISABLE_SUPPORT_EXPONENTIAL
// #define PRINTF_SUPPORT_EXPONENTIAL
// #endif

// define the default floating point precision
// default: 6 digits
#ifndef PRINTF_DEFAULT_FLOAT_PRECISION
#define PRINTF_DEFAULT_FLOAT_PRECISION  0U
#endif

// define the largest float suitable to print with %f
// default: 1e9
#ifndef PRINTF_MAX_FLOAT
#define PRINTF_MAX_FLOAT  0
#endif

// support for the long long types (%llu or %p)
// default: activated
#define PRINTF_DISABLE_SUPPORT_LONG_LONG
// #ifndef PRINTF_DISABLE_SUPPORT_LONG_LONG
// #define PRINTF_SUPPORT_LONG_LONG
// #endif

// support for the ptrdiff_t type (%t)
// ptrdiff_t is normally defined in <stddef.h> as long or long long type
// default: activated

#define PRINTF_DISABLE_SUPPORT_PTRDIFF_T
// #ifndef PRINTF_DISABLE_SUPPORT_PTRDIFF_T
// #define PRINTF_SUPPORT_PTRDIFF_T
// #endif