#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <clib/debug_protos.h>

#ifdef NDBEUG
#define LOG(...)
#else
#define LOG(...) KPrintF(__VA_ARGS__)
#endif

#endif
