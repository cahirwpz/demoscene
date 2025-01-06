#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
 * Defined:
 *   Use fs-uae dependant features i.e. call-traps (see `include/uae.h`)
 *   to aid debugging and profiling. This may render executable files
 *   unusable (most likely crash) on real hardware.
 * Not defined:
 *   Disable use of aforementioned features. Required for a release!
 */
#define UAE

#ifndef UAE
/*
 * Redirect diagnostic and log messages to:
 * 0 => null output (drop messages)
 * 1 => parallel port
 * 2 => serial port
 */
#define LOGOUT 2
#endif

/*
 * Defined:
 *   Make executable files compatible with AmigaOS. Created ADFs will
 *   be formatted with Old Filesystem (KS1.3) and contain special bootblock
 *   that maximizes amount of chip memory. Executable file will be started
 *   automatically facilitating `startup-sequence` feature of AmigaDOS.
 * Not defined:
 *   Executable files must be started from ADF since they require
 *   custom environment created by bootstrap code.
 */
/* #define AMIGAOS */

/*
 * Turn on profiler that reports minimum-average-maximum number of raster
 * lines measured between calls to ProfilerStart / ProfilerStop.
 * The measurement is reported every 50 frames (i.e. once a second).
 */
#define PROFILER

/*
 * Enable multitasking feature. It can be necessary to run background thread
 * that loads data from disk while the effects are running.
 */
#define MULTITASK

/*
 * Enable dynamic memory allocation debugging incl. nice diagnostic printout
 * when memory corruption is detected or we run out of memory.
 */
#define MEMDEBUG

/*
 * [Valid only when AMIGAOS is defined!]
 * Amount of chip and fast (or public) memory
 * (in kilobytes!) passed to our custom memory allocator.
 * To calculate total memory taken after an executable file
 * is loaded into memory please use `m68k-amigaos-objdump` tool.
 */
#define CHIPMEM 160
#define FASTMEM 288

#endif /* !__CONFIG_H__ */
