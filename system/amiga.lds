/* Linker scripts are documented at:
 * https://sourceware.org/binutils/docs/ld/Scripts.html */
OUTPUT_FORMAT("amiga")
OUTPUT_ARCH(m68k)
PROVIDE(_SysBase = 0x4);
PROVIDE(__ciaa = 0xbfe001);
PROVIDE(__ciab = 0xbfd000);
PROVIDE(__custom = 0xdff000);
SECTIONS
{
  . = 0x0;
  .text :
  {
    __text = .;
    *(.text)
    *(.text.*)
  }
  __text_size = SIZEOF(.text);
  . = ALIGN(0x0);
  .data :
  {
    __data = .;
    CONSTRUCTORS
    *(.data)
    *(.data.*)
  }
  __data_size = SIZEOF(.data);
  .bss :
  {
    __bss = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
  }
  __bss_size = SIZEOF(.bss);
  .datachip :
  {
    __data_chip = .;
    *(.datachip)
    *(.datachip.*)
  }
  __data_chip_size = SIZEOF(.datachip);
  .bsschip :
  {
    __bss_chip = .;
    *(.bsschip)
    *(.bsschip.*)
  }
  __bss_chip_size = SIZEOF(.bsschip);
  .stab :
  {
    *(.stab)
  }
  .stabstr :
  {
    *(.stabstr)
  }
}
