#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef NDEBUG
#define LOG(...)
#else
#include <stdio.h>
#define LOG(...) printf(__VA_ARGS__)
#endif

#endif
