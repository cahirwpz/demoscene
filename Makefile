TOPDIR = $(realpath .)

SUBDIRS = tools lib system effects demo
CLEAN-FILES = bootloader.bin addchip.bootblock.bin vbrmove

all: bootloader.bin addchip.bootblock.bin vbrmove build

addchip.bootblock.bin: VASMFLAGS += -phxass -cnop=0

include $(TOPDIR)/build/common.mk

gdb-dashboard:
	wget -O $@ https://raw.githubusercontent.com/cyrus-and/gdb-dashboard/master/.gdbinit
