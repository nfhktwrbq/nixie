#pragma once

#include "printf/printf.h"


#ifdef DEBUG
#define DEBUG_LEVEL_ERR     (1u)
#define DEBUG_LEVEL_WRN     (2u)
#define DEBUG_LEVEL_INFO    (3u)

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_LEVEL_INFO
#endif

#if (DEBUG_LEVEL >= DEBUG_LEVEL_ERR)
#define DBG_ERR(...) printf( __VA_ARGS__ )
#endif

#if (DEBUG_LEVEL >= DEBUG_LEVEL_WRN)
#define DBG_WRN(...) printf( __VA_ARGS__ )
#endif

#if (DEBUG_LEVEL >= DEBUG_LEVEL_INFO)
#define DBG_INFO(...) printf( __VA_ARGS__ )
#endif

#else
#define DEBUG_PRINT(...) do{ } while ( 0 )
#define DBG_ERR(...) do{ } while ( 0 )
#define DBG_WRN(...) do{ } while ( 0 )
#define DBG_INFO(...) do{ } while ( 0 )
#endif