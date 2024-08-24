#!/usr/bin/env python3

import argparse
import os
from array import array
from struct import pack
from io import BytesIO

from fsutil import SECTOR, write_pad, sectors, Filesystem

#
# On disk format description:
#
# sector 0..1: bootblock
#  [LONG] 'DOS\0'
#  [LONG] checksum
#  [WORD] start of executable file, sector aligned, shifted right by 8
#  [WORD] offset of executable file, sector aligned, shifte right by 8
#  ...    boot code
#
# sector 2..: file system image
#

FLOPPY = SECTOR * 80 * 11 * 2


def checksum(data):
    arr = array('I', data)
    arr.byteswap()
    chksum = sum(arr)
    chksum = (chksum >> 32) + (chksum & 0xffffffff)
    return (~chksum & 0xffffffff)


def write_bb(adf, bootcode, exe):
    boot = BytesIO(bootcode)
    # Overwrite boot block header
    exe_start = sectors(exe.offset) + 2
    exe_length = sectors(exe.size)
    boot.write(pack('>4s4xHH', b'DOS\0', exe_length * 2, exe_start * 2))
    # Move to the end and pad it so it takes 2 sectors
    boot.seek(0, os.SEEK_END)
    write_pad(boot, 2 * SECTOR)
    # Calculate checksum and fill missing field in boot block
    val = checksum(boot.getvalue())
    boot.seek(4, os.SEEK_SET)
    boot.write(pack('>I', val))
    # Write fixed boot block to file system image
    adf.write(boot.getvalue())


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Create ADF file from file system image.')
    parser.add_argument(
        '-b', '--bootcode', metavar='BOOTCODE', type=str,
        help='Code to be put into boot block')
    parser.add_argument(
        'image', metavar='IMAGE', type=str,
        help='File system image file')
    parser.add_argument(
        'adf', metavar='ADF', type=str,
        help='ADF output file')
    args = parser.parse_args()

    bootblock = ''
    executable = None

    if args.bootcode:
        if not os.path.isfile(args.bootcode):
            raise SystemExit('Boot code file does not exists!')
        if os.path.getsize(args.bootcode) > 2 * SECTOR:
            raise SystemExit('Boot code is larger than 1024 bytes!')
        with open(args.bootcode, 'rb') as fh:
            bootblock = fh.read()

        executable = Filesystem.find_exec(args.image)
        if not executable:
            raise SystemExit('No AmigaHunk executable found!')

    with open(args.image, 'rb') as img:
        img = img.read()

    if len(img) > FLOPPY - 2 * SECTOR:
        img_len = len(img) // 1024
        raise SystemExit(f'Image is to big to be written to a disk ({img_len} kB)!')

    with open(args.adf, 'wb') as adf:
        if bootblock:
            write_bb(adf, bootblock, executable)

        # Write file system image
        adf.write(img)

        # Complete floppy disk image
        write_pad(adf, FLOPPY)
