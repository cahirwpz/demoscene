# 1 => Use fs-uae dependant features i.e. call-traps (see `include/uae.h`)
#      to aid debugging and profiling. This may render executable files
#      unusable (most likely crash) on real hardware.
# 0 => Disable use of aforementioned features. Required for a release!
UAE := 1

# [only when UAE=0] Redirect diagnostic and log messages to:
# 0 => null output (drop messages)
# 1 => parallel port
# 2 => serial port
LOGOUT := 2

# 1 => Make executable files compatible with AmigaOS. Created ADFs will
#      be formatted with Old Filesystem (KS1.3) and contain special bootblock
#      that maximizes amount of chip memory. Executable file will be started
#      automatically facilitating `startup-sequence` feature of AmigaDOS.
# 0 => Executable files must be started from ADF since they require
#      custom environment created by bootstrap code.
AMIGAOS := 0

# 1 => Turn on profiler that reports minimum-average-maximum number of raster
#      lines measured between calls to ProfilerStart / ProfilerStop.
#      The measurement is reported every 50 frames (i.e. once a second).
PROFILER := 1

# 1 => Enable multitasking feature. It can be necessary to run background thread
#      that loads data from disk while the effects are running.
MULTITASK := 1

# 1 => Enable dynamic memory allocation debugging incl. nice diagnostic printout
#      when memory corruption is detected or we run out of memory.
MEMDEBUG := 1

# [only when AMIGAOS=1] Amount of chip and fast (or public) memory 
# (in kilobytes!) passed to our custom memory allocator.
# To calculate total memory taken after an executable file
# is loaded into memory please use `m68k-amigaos-objdump` tool.
CHIPMEM := 160
FASTMEM := 288

# Pass "VERBOSE=1" at command line to display command being invoked by GNU Make
VERBOSE ?= 0
