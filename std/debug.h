#ifndef __STD_DEBUG_H__
#define __STD_DEBUG_H__

#ifdef NDEBUG
#define LOG(...)
#define PANIC(...) _exit();
#define ASSERT(...)
#else
#include <stdio.h>
#define _LINE() printf("%s:%d: ", __FILE__, __LINE__)
#define _EOL() printf("\r\n")
#define LOG(...) { _LINE(); printf(__VA_ARGS__); _EOL(); }
#define PANIC(...) { LOG(__VA_ARGS__); _exit(); }
#define ASSERT(cond, ...) { if (!(cond)) { LOG(__VA_ARGS__); _exit(); } }
#endif

#endif
