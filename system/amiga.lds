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
    *(.rodata)
    *(.rodata.*)
  }
  __text_size = SIZEOF(.text);
  . = ALIGN(8);
  .data :
  {
    __data = .;
    CONSTRUCTORS
    *(.data)
    *(.data.*)
  }
  __data_size = SIZEOF(.data);
  . = ALIGN(8);
  .bss :
  {
    __bss = .;
    *(.bss)
    *(.bss.*)
    *(COMMON)
  }
  __bss_size = SIZEOF(.bss);
  . = ALIGN(8);
  .datachip :
  {
    __data_chip = .;
    *(.datachip)
    *(.datachip.*)
  }
  __data_chip_size = SIZEOF(.datachip);
  . = ALIGN(8);
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
