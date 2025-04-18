#ifndef __UAE_H__
#define __UAE_H__

/*
 * Corresponding interface is implemented in fs-uae:
 * procedure uaelib_demux_common in src/uaelib.cpp file
 */

int _UaeTrap(int i, ...);
int _UaeLog(int i, const char *format, ...)
  __attribute__ ((format (printf, 2, 3)));
int _UaeWarpMode(int i, int val);

enum {
  uaelib_HardReset = 3,
  uaelib_Reset = 4,
  uaelib_ExitEmu = 13,
  uaelib_Log = 40,
  uaelib_WarpMode = 41,
};

#define UaeHardReset() _UaeTrap(uaelib_HardReset)
#define UaeReset() _UaeTrap(uaelib_Reset)
#define UaeExit() _UaeTrap(uaelib_ExitEmu)
#define UaeLog(...) _UaeLog(uaelib_Log, __VA_ARGS__)
#define UaeWarpMode(val) _UaeTrap(uaelib_WarpMode, val);

#endif /* !__UAE_H__ */
