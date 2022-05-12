/* Linker scripts are documented at:
 * https://sourceware.org/binutils/docs/ld/Scripts.html */
OUTPUT_FORMAT("amiga")
OUTPUT_ARCH(m68k)
PROVIDE(_SysBase = 0x4);
PROVIDE(__ciaa = 0xbfe001);
PROVIDE(__ciab = 0xbfd000);
PROVIDE(__custom = 0xdff000);
INCLUDE system-api.lds
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
