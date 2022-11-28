# some generic settings
set output-radix 0x10
set pagination off
set confirm off
set verbose off

source gdb-dashboard
dashboard -layout breakpoints assembly registers source stack variables
# Please refer to https://github.com/cyrus-and/gdb-dashboard/wiki
# to learn how to customize visual experience
dashboard -style style_low '2'

# Use `monitor` command to access fs-uae debugger.
# Type `monitor ?` to see the list of available commands.
# Some useful commands are:
# * `v -3` - turn on visual DMA debugger
# * `vo` - turn off visual DMA debugger
# * `o 1` - display and disassembly current copper list
# * `c` - display state of CIA registers
# * `e` - display state of custom registers

# Please use fs-uae debugger carefully - preferably only for inspecting state of
# the machine. It's a very powerful tool but even mistyping a command can break
# a session between gdb and the emulator.
