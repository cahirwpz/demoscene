#!/usr/bin/env python3

import argparse
import json
from string import Template


LinkerScript = Template("""
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
    ${excluded} *(.text)
    *(.text.*)
  }
  __text_size = SIZEOF(.text);
  . = ALIGN(0x0);
  .data :
  {
    __data = .;
    CONSTRUCTORS
    ${excluded} *(.data)
  }
  __data_size = SIZEOF(.data);
  .bss :
  {
    __bss = .;
    ${excluded} *(.bss)
    *(COMMON)
  }
  __bss_size = SIZEOF(.bss);
  .datachip :
  {
    __data_chip = .;
    ${excluded} *(.datachip)
  }
  __data_chip_size = SIZEOF(.datachip);
  .bsschip :
  {
    __bss_chip = .;
    ${excluded} *(.bsschip)
  }
  __bss_chip_size = SIZEOF(.bsschip);
  ${effects}
}""")

PerEffect = Template("""
  /* ${name} effect */

  .text.${name} :
  {
    ${objects} (.text .text.*)
  }
  .data.${name} :
  {
    ${objects} (.data)
  }
  .bss.${name} :
  {
    ${objects} (.bss)
  }
  .datachip.${name} :
  {
    ${objects} (.datachip)
  }
  .bsschip.${name} :
  {
    ${objects} (.bsschip)
  }
""")

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Prepare linker script to split out loadable effects.')
    parser.add_argument('input', metavar='INPUT', type=str,
                        help='JSON file with list of object files per effect.')
    parser.add_argument('output', metavar='OUTPUT', type=str,
                        help='GNU linker script file.')
    args = parser.parse_args()

    with open(args.input) as f:
        loadables = json.load(f)

    objects = set()
    effects = []
    for k, v in loadables.items():
        effects.append(PerEffect.substitute(name=k, objects=" ".join(v)))
        objects = objects.union(set(v))

    if objects:
        excluded = 'EXCLUDED_FILE(%s)' % " ".join(objects)
    else:
        excluded = ''

    lds = LinkerScript.substitute(effects="".join(effects),
                                  excluded=excluded)

    with open(args.output, "w") as f:
        f.write(lds)
