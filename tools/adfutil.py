#!/usr/bin/env python3

import argparse
import os
from array import array
from struct import pack
from io import BytesIO

from fsutil import SECTOR, align, write_pad, sectors, Filesystem

#
# On disk format description:
#
# sector 0..1: bootblock
#  [LONG] 'DOS\0'
#  [LONG] checksum
#  [LONG] size of executable file aligned to sector size (takes m sectors)
#  ...    boot code
#
# sector 2..: file system image

FLOPPY = SECTOR * 80 * 11 * 2


def checksum(data):
    arr = array('I', data)
    arr.byteswap()
    chksum = sum(arr)
    chksum = (chksum >> 32) + (chksum & 0xffffffff)
    return (~chksum & 0xffffffff)


def bootcode(path):
    # Collect boot code if there's any...
    if path is None:
        return ''

    if not os.path.isfile(path):
        raise SystemExit('Boot code file does not exists!')

    with open(path, 'rb') as fh:
        bootcode = fh.read()
    if len(bootcode) > 2 * SECTOR:
        raise SystemExit('Boot code is larger than 1024 bytes!')
    return bootcode


def find_exec(img):
    with open(img, 'rb') as fs:
        for entry in Filesystem.load(fs):
            if entry.exe:
                return entry

    return None


def write_bb(adf, bootcode, exe):
    boot = BytesIO(bootcode)
    # Overwrite boot block header
    exe_start = sectors(exe.offset)
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

    bootblock = bootcode(args.bootcode)
    executable = None

    if bootblock:
        executable = find_exec(args.image)
        if not executable:
            raise SystemExit('No AmigaHunk executable found!')

    with open(args.adf, 'wb') as adf:
        if bootblock:
            write_bb(adf, bootblock, executable)

        # Write file system image
        with open(args.image, 'rb') as img:
            adf.write(img.read())

        # Complete floppy disk image
        write_pad(adf, FLOPPY)
