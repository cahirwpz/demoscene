/* Linker scripts are documented at:
 * https://sourceware.org/binutils/docs/ld/Scripts.html */
OUTPUT_FORMAT("amiga")
OUTPUT_ARCH(m68k)
PROVIDE(_SysBase = 0x4);
PROVIDE(__ciaa = 0xbfe001);
PROVIDE(__ciab = 0xbfd000);
PROVIDE(__custom = 0xdff000);
PROVIDE(__MemAlloc = 0xc0);
PROVIDE(__MemResize = 0xc6);
PROVIDE(__MemFree = 0xcc);
PROVIDE(__AddIntServer = 0xd2);
PROVIDE(__RemIntServer = 0xd8);
PROVIDE(___ProfilerStart = 0xde);
PROVIDE(___ProfilerStop = 0xe4);
PROVIDE(__TaskWaitVBlank = 0xea);
PROVIDE(_exitLoop = 0x1f7);
PROVIDE(_frameCount = 0x1f8);
PROVIDE(_lastFrameCount = 0x1fc);
SECTIONS
{
  . = 0x0;
  .text :
  {
    __text = .;
    *(.text)
  }
  __text_size = SIZEOF(.text);
  . = ALIGN(0x0);
  .data :
  {
    __data = .;
    CONSTRUCTORS
    *(.data)
  }
  __data_size = SIZEOF(.data);
  .bss :
  {
    __bss = .;
    *(.bss)
    *(COMMON)
  }
  __bss_size = SIZEOF(.bss);
  .data_chip :
  {
    __data_chip = .;
    *(.data_chip)
  }
  __data_chip_size = SIZEOF(.data_chip);
  .bss_chip :
  {
    __bss_chip = .;
    *(.bss_chip)
  }
  __bss_chip_size = SIZEOF(.bss_chip);
}
