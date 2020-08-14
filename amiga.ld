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
    __stext = .;
    *(.text)
    ___datadata_relocs = .;
    __etext = .;
  }
  ___text_size = SIZEOF(.text);
  . = ALIGN(0x0);
  .data :
  {
    __sdata = .;
    CONSTRUCTORS
    *(.data)
    ___a4_init = 0x7ffe;
    __edata = .;
  }
  ___data_size = SIZEOF(.data);
  .bss :
  {
    __bss_start = .;
    *(.bss)
    *(COMMON)
    __end = .;
  }
  ___bss_size = SIZEOF(.bss);
  .data_chip :
  {
    *(.data_chip)
  }
  .bss_chip :
  {
    *(.bss_chip)
  }
}
