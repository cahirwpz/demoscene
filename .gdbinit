# some generic settings
set output-radix 0x10
set pagination off
set confirm off
set verbose off

source gdb-dashboard
dashboard -layout breakpoints assembly registers source stack variables
dashboard -style style_low '2'
