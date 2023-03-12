# some generic settings
set output-radix 0x10
set pagination off
set confirm off
set verbose off

source gdb-dashboard
dashboard -layout breakpoints assembly registers source stack variables
# Please refer to https://github.com/cyrus-and/gdb-dashboard/wiki to learn how to customize visual experience
dashboard -style style_low '2'
