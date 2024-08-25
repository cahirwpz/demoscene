#!/usr/bin/env python3

import argparse
import json
from contextlib import redirect_stdout
from string import Template
from io import StringIO


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


Sections = [('.text', '.text .text.*'), ('.data', '.data'), ('.bss', '.bss'),
            ('.datachip', '.datachip'), ('.bsschip', '.bsschip')]


def per_effect(name: str, objects: list[str]) -> str:
    f = StringIO()

    with redirect_stdout(f):
        print(f"  /* {name} effect */")
        print()

        for dst, src in Sections:
            print(f"  {dst}.{name} :")
            print("  {")
            for object in objects:
                print(f"    {object} ({src})")
            print("  }")

    return f.getvalue()


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
        effects.append(per_effect(k, v))
        objects = objects.union(set(v))

    if objects:
        excluded = 'EXCLUDE_FILE(%s)' % " ".join(objects)
    else:
        excluded = ''

    lds = LinkerScript.substitute(effects="".join(effects),
                                  excluded=excluded)

    with open(args.output, "w") as f:
        f.write(lds)
