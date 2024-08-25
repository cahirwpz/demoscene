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
ROMFOOTER = 16


def write_startup(rom, startup, exe):
    startup = BytesIO(startup)
    # Overwrite rom startup hunk file setup
    startup.seek(8, os.SEEK_SET)
    startup.write(pack('>II', exe.offset + 2 * SECTOR + ROMADDR, exe.size))
    # Move to the end and pad it so it takes 2 sectors
    startup.seek(0, os.SEEK_END)
    write_pad(startup, 2 * SECTOR)
    # Write startup to ROM image
    rom.write(startup.getvalue())


def write_footer(rom):
    rom.seek(-ROMFOOTER, os.SEEK_END)
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

    startup = None

    if not os.path.isfile(args.startup):
        raise SystemExit('ROM startup code file does not exists!')
    if os.path.getsize(args.startup) > 2 * SECTOR:
        raise SystemExit('ROM startup code is larger than 1024 bytes!')
    with open(args.startup, 'rb') as fh:
        startup = fh.read()

    executable = Filesystem.find_exec(args.image)

    if not executable:
        raise SystemExit('No AmigaHunk executable found!')

    rom = BytesIO()
    write_startup(rom, startup, executable)

    # Startup and executable must fit into first 512kB
    assert rom.tell() <= ROMSIZE - ROMFOOTER

    # Read file system image
    with open(args.image, 'rb') as f:
        img = f.read()

    # 1024kB ROM is tricky to handle: first half is mapped at
    # 0xe00000, second half is mapped at 0xf80000 and should
    # look like normal Kickstart ROM

    if rom.tell() + len(img) > ROMSIZE - ROMFOOTER:
        # 1024kB: Complete ROM disk image
        hi_size = ROMSIZE - ROMFOOTER - rom.tell()
        lo_size = len(img) - hi_size
        rom.write(img[:hi_size])
        write_pad(rom, ROMSIZE)
        write_footer(rom)
        rom.write(img[hi_size:])
        write_pad(rom, ROMSIZE)

        with open(args.rom, 'wb') as f:
            # swap halves
            f.write(rom.getvalue()[ROMSIZE:ROMSIZE*2])
            f.write(rom.getvalue()[:ROMSIZE])
    else:
        # 512kB: Complete ROM disk image
        rom.write(img)
        write_pad(rom, ROMSIZE)
        write_footer(rom)

        with open(args.rom, 'wb') as f:
            f.write(rom.getvalue())
