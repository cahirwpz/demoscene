#!/usr/bin/env python3

import argparse
import os
import stat
from collections import UserList
from fnmatch import fnmatch
from struct import pack, unpack
from zlib import crc32

#
# On disk format description:
#
# sector 0..(n-1): directory entries (take n sectors)
#  [WORD] dirsize : total size of directory entries in bytes
#  for each directory entry (2-byte aligned):
#   [BYTE] #reclen : total size of this record
#   [BYTE] #type   : type of file (1: executable, 0: regular)
#   [WORD] #start  : sector where the file begins (0..1759)
#   [LONG] #length : size of the file in bytes (up to 1MiB)
#   [LONG] #cksum  : crc32 checksum on file data
#   [STRING] #name : name of the file (NUL terminated)
#
# sector (n)..(n+k-1): content of files
#

SECTOR = 512


def align(size, alignment=None):
    if alignment is None:
        alignment = SECTOR
    return (size + alignment - 1) // alignment * alignment


def sectors(size):
    return align(size, SECTOR) // SECTOR


def write_pad(fh, alignment=None):
    pos = fh.tell()
    pad = align(pos, alignment) - pos
    fh.write(b'\0' * pad)


class FileEntry(object):
    __slots__ = ('name', 'offset', 'exe', 'size', 'cksum', 'data')

    def __init__(self, name, offset, exe, size, cksum, data=None):
        self.name = name
        self.offset = offset
        self.exe = exe
        self.size = size
        self.cksum = cksum
        self.data = data

    def __str__(self):
        s = '%-32s %6d' % (self.name, self.size)
        if self.exe:
            s += ' (executable)'
        return s


class Filesystem(UserList):
    @classmethod
    def load(cls, fh):
        dir_len = unpack('>H', fh.read(2))[0]

        entries = []
        while dir_len > 0:
            reclen, exe, offset, size, cksum = unpack('>BBHII', fh.read(12))
            name = fh.read(reclen - 12).decode().rstrip('\0')
            dir_len -= reclen
            entries.append(FileEntry(name, offset * SECTOR, exe, size, cksum))

        for entry in entries:
            fh.seek(entry.offset)
            entry.data = fh.read(entry.size)

        return cls(entries)

    @classmethod
    def make(cls, paths):
        entries = []
        file_off = 0

        for path in paths:
            if not os.path.exists(path):
                raise SystemExit('create: %s does not exist' % path)
            if not os.path.isfile(path):
                raise SystemExit('create: %s is not a regular file' % path)

            name = os.path.basename(path)

            if any(e.name == name for e in entries):
                raise SystemExit(
                    'create: %s has already been added to archive' % path)

            with open(path, 'rb') as fh:
                data = fh.read()

            exe = bool(os.stat(path).st_mode & stat.S_IEXEC)
            entry = FileEntry(name, file_off, exe, len(data), crc32(data), data)
            entries.append(entry)

            # Determine file position
            file_off += align(entry.size)

        return cls(entries)

    def save(self, path):
        # Determine directory size
        dir_len = sum(align(12 + len(entry.name) + 1, 2) for entry in self.data)

        # Calculate starting position of files in the file system image
        files_pos = align(dir_len)

        with open(path, 'wb') as fh:
            # Write directory header
            fh.write(pack('>H', dir_len))
            # Write directory entries
            for entry in self.data:
                start = sectors(entry.offset + files_pos)
                reclen = align(12 + len(entry.name) + 1, 2)
                name = entry.name.encode('ascii') + b'\0'
                is_exe = int(entry.exe)
                fh.write(pack('>BBHII%ds' % len(name),
                              reclen, is_exe, start,
                              entry.size, entry.cksum, name))
                write_pad(fh, 2)
            # Finish off directory by aligning it to sector boundary
            write_pad(fh)

            # Write file entries
            for entry in self.data:
                fh.write(entry.data)
                write_pad(fh)

    @classmethod
    def find_exec(cls, path):
        with open(path, 'rb') as fs:
            for entry in cls.load(fs):
                if entry.exe:
                    return entry

        return None


def extract(archive, patterns, force):
    for pattern in patterns:
        for entry in archive:
            if fnmatch(entry.name, pattern):
                if os.path.exists(entry.name) and not force:
                    print(f'extract: skipping {entry.name} '
                          '- file is present on disk')
                else:
                    print(f'extracting {entry.name}')
                    with open(entry.name, 'wb') as fh:
                        fh.write(entry.data)
                    os.chmod(entry.name, [0o644, 0o755][entry.exe])


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Tool for handling read-only file system images.')
    parser.add_argument(
        'action', metavar='ACTION', type=str,
        choices=['create', 'extract', 'list'],
        help='Action to perform of filesystem image.')
    parser.add_argument(
        '-f', '--force', action='store_true',
        help='If output file exist, the tool will overwrite it.')
    parser.add_argument(
        'image', metavar='IMAGE', type=str,
        help='File system image file.')
    parser.add_argument(
        'files', metavar='FILES', type=str, nargs='*',
        help='Files to add to / extract from filesystem image.')
    args = parser.parse_args()

    if args.action == 'create':
        archive = Filesystem.make(args.files)
        for entry in archive:
            print(entry)
        archive.save(args.image)
    elif args.action == 'list':
        with open(args.image, 'rb') as fh:
            archive = Filesystem.load(fh)
            for entry in archive:
                print(entry)
    elif args.action == 'extract':
        with open(args.image, 'rb') as fh:
            archive = Filesystem.load(fh)
            extract(archive, args.files, args.force)
