#ifndef _UAE_H_
#define _UAE_H_

/*
 * Corresponding interface is implemented in fs-uae:
 * procedure uaelib_demux_common in src/uaelib.cpp file
 */

int _UaeTrap(int i, ...);
int _UaeLog(int i, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));

enum {
  uaelib_HardReset = 3,
  uaelib_Reset = 4,
  uaelib_ExitEmu = 13,
  uaelib_Log = 40,
};

#define UaeHardReset() _UaeTrap(uaelib_HardReset)
#define UaeReset() _UaeTrap(uaelib_Reset)
#define UaeExit() _UaeTrap(uaelib_ExitEmu)
#define UaeLog(...) _UaeLog(uaelib_Log, __VA_ARGS__)

#endif
