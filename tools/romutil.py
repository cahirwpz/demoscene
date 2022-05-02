#!/usr/bin/env python3

import argparse
import os
from array import array
from struct import pack
from io import BytesIO

from fsutil import SECTOR, write_pad, Filesystem

#
# In memory format description:
#
# sector 0..1: startup code
#  [LONG] initial stack pointer
#  [LONG] initial program counter
#  [LONG] ROM address of executable file, sector aligned
#  [LONG] size of executable file, in bytes
#  ...    startup code
#
# sector 2..: file system image
#

ROMADDR = 0xf80000
ROMSIZE = 0x080000


def romcode(path):
    if not os.path.isfile(path):
        return None
    with open(path, 'rb') as fh:
        romcode = fh.read()
    if len(romcode) > 2 * SECTOR:
        raise SystemExit('ROM startup code is larger than 1024 bytes!')
    return romcode


def write_startup(rom, startup, exe):
    startup = BytesIO(startup)
    # Overwrite rom startup hunk file setup
    startup.seek(8, os.SEEK_SET)
    startup.write(pack('>II', exe.offset + 2 * SECTOR + ROMADDR, exe.size))
    # Move to the end and pad it so it takes 2 sectors
    startup.seek(0, os.SEEK_END)
    write_pad(startup, 2 * SECTOR)
    # Write fixed boot block to file system image
    rom.write(startup.getvalue())


def write_footer(rom):
    rom.seek(-16, os.SEEK_END)
    rom.write(bytes.fromhex('471848194f1a531b541c4f1d571e4e1f'))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Create ROM file from file system image.')
    parser.add_argument(
        'startup', metavar='STARTUP', type=str,
        help='ROM startup code')
    parser.add_argument(
        'image', metavar='IMAGE', type=str,
        help='File system image file')
    parser.add_argument(
        'rom', metavar='ROM', type=str,
        help='ROM output file')
    args = parser.parse_args()

    startup = romcode(args.startup)
    if not startup:
        raise SystemExit('ROM startup code file does not exists!')
    executable = Filesystem.find_exec(args.image)
    if not executable:
        raise SystemExit('No AmigaHunk executable found!')

    with open(args.rom, 'wb') as rom:
        write_startup(rom, startup, executable)

        # Write file system image
        with open(args.image, 'rb') as img:
            rom.write(img.read())

        # Complete ROM disk image
        write_pad(rom, ROMSIZE)
        write_footer(rom)
