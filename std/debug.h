#ifndef __STD_DEBUG_H__
#define __STD_DEBUG_H__

#include <stdio.h>

#include "std/exception.h"

#ifdef NDEBUG
#define LOG(...)
#define PANIC(...) RAISE
#define ASSERT(cond, ...) { if (!(cond)) { RAISE; } }
#else
#define _LINE() printf("%s:%d: ", __FILE__, __LINE__)
#define _EOL() printf("\r\n")
#define LOG(...) { _LINE(); printf(__VA_ARGS__); _EOL(); }
#define PANIC(...) { LOG(__VA_ARGS__); RAISE; }
#define ASSERT(cond, ...) { if (!(cond)) { LOG(__VA_ARGS__); RAISE; } }
#endif

#endif
