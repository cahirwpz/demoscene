#!/usr/bin/env python3

import argparse
import os
import stat
from array import array
from fnmatch import fnmatch
from struct import pack, unpack
from io import BytesIO

#
# On disk format description:
#
# sector 0..1: bootblock
#  [LONG] 'DOS\0'
#  [LONG] checksum
#  [LONG] size of directory entries (aligned to sector size)
#  ...    boot code
#
# sector 2..(n-1): directory entries
#  [WORD] #files   : number of file entries
#  [WORD] #names   : filenames blob length in bytes
#  repeated #files times:
#   [LONG] #start  : byte position where file begins
#   [LONG] #length : size of the file
#
#  If #start is negative then the file is executable. One needs to take its
#  negative to recover original value.
#
# sector n..(m-1): filenames blob
#  stores consecutively #files strings with trailing '\0'
#
# sector m..1759: data
#

SECTOR = 512
FLOPPY = SECTOR * 80 * 11 * 2
ALIGNMENT = 4


def align(size, alignment=None):
  if alignment is None:
    alignment = ALIGNMENT
  return (size + alignment - 1) // alignment * alignment


def skip_pad(fh, alignment=None):
  pos = fh.tell()
  fh.seek(align(pos, alignment) - pos, 1)


def write_pad(fh, alignment=None):
  pos = fh.tell()
  fh.write(b'\0' * (align(pos, alignment) - pos))


def checksum(data):
  arr = array('I', data)
  arr.byteswap()
  chksum = sum(arr)
  chksum = (chksum >> 32) + (chksum & 0xffffffff)
  return (~chksum & 0xffffffff)


class FileEntry(object):
  __slots__ = ('name', 'data', 'exe')

  def __init__(self, name, data, exe):
    self.name = name
    self.data = data
    self.exe = exe

  def __str__(self):
    s = '%-32s %6d' % (self.name, len(self.data))
    if self.exe:
      s += ' (executable)'
    return s


def collect(paths):
  entries = []

  for path in paths:
    if not os.path.exists(path):
      raise SystemExit('create: %s does not exist' % path)
    if not os.path.isfile(path):
      raise SystemExit('create: %s is not a regular file' % path)

    name = os.path.basename(path)

    if any(e.name == name for e in entries):
      raise SystemExit('create: %s has already been added to archive' % path)

    with open(path, 'rb') as fh:
      data = fh.read()

    entry = FileEntry(name, data, bool(os.stat(path).st_mode & stat.S_IEXEC))
    entries.append(entry)

  return entries


def load(archive, floppy):
  with open(archive, 'rb') as fh:
    if floppy:
      fh.seek(2 * SECTOR)

    dirent_count, names_len = unpack('>HH', fh.read(4))
    dirent = [list(unpack('>iI', fh.read(8))) for i in range(dirent_count)]
    skip_pad(fh)

    names = fh.read(names_len + 1)
    skip_pad(fh)

    pos = 0
    for entry in dirent:
      end = pos
      while names[end] != '\0':
        end += 1
      entry.append(names[pos:end])
      pos = end + 1

    entries = []
    for offset, size, name in dirent:
      exe = bool(offset < 0)
      if exe:
        offset = -offset
      fh.seek(offset)
      entries.append(FileEntry(name, fh.read(size), exe))

    return entries


def save(archive, floppy, entries, bootcode=None):
  if bootcode is not None:
    if os.path.isfile(bootcode):
      with open(bootcode, 'rb') as fh:
        bootcode = fh.read()
      if len(bootcode) > 2 * SECTOR:
        raise SystemExit('Boot code is larger than 1024 bytes!')
    else:
      raise SystemExit('Boot code file does not exists!')
  else:
    bootcode = ''

  offsets = []
  dirent_len = 4 + 8 * len(entries)
  names_len = 0
  files_len = 0

  for entry in entries:
    offsets.append(files_len)
    names_len += len(entry.name) + 1
    files_len += align(len(entry.data))

  files_pos = align(dirent_len) + align(names_len)

  if floppy:
    files_pos += 2 * SECTOR

  with open(archive, 'wb') as fh:
    if floppy:
      boot = BytesIO(bootcode)
      boot.write(pack('>4s4xI', b'DOS\0', align(dirent_len)))
      boot.seek(0, 2)
      write_pad(boot, 2 * SECTOR)
      val = checksum(boot.getvalue())
      boot.seek(4, 0)
      boot.write(pack('>I', val))
      fh.write(boot.getvalue())

    fh.write(pack('>HH', len(entries), names_len))
    for entry, file_pos in zip(entries, offsets):
      file_pos += files_pos
      if entry.exe:
        file_pos = -file_pos
      fh.write(pack('>iI', file_pos, len(entry.data)))
    write_pad(fh)

    for entry in entries:
      fh.write(entry.name.encode())
      fh.write(b'\0')
    write_pad(fh)

    for entry in entries:
      fh.write(entry.data)
      write_pad(fh)

    if floppy:
      write_pad(fh, FLOPPY)


def extract(archive, patterns, force):
  for pattern in patterns:
    for entry in archive:
      if fnmatch(entry.name, pattern):
        if os.path.exists(entry.name) and not force:
          print('extract: skipping %s - file is present on disk' % entry.name)
        else:
          print('extracting %s' % entry.name)
          with open(entry.name, 'wb') as fh:
            fh.write(entry.data)
          os.chmod(entry.name, [0o644, 0o755][entry.exe])


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
      description='Tool for handling Read-Only FileSystem.')
  parser.add_argument(
      'action', metavar='ACTION', type=str,
      choices=['create', 'extract', 'list'],
      help='Action to perform of filesystem image.')
  parser.add_argument(
      '-f', '--force', action='store_true',
      help='If output file exist, the tool will overwrite it.')
  parser.add_argument(
      '--floppy', action='store_true',
      help='Filesystem image will be tuned to disk floppy representation.')
  parser.add_argument(
      '-b', '--bootcode', metavar='BOOTCODE', type=str,
      help='Boot code to be embedded into floppy disk representation.')
  parser.add_argument(
      'image', metavar='IMAGE', type=str,
      help='FileSystem image file.')
  parser.add_argument(
      'files', metavar='FILES', type=str, nargs='*',
      help='Files to add to / extract from filesystem image.')
  args = parser.parse_args()

  if args.bootcode:
    args.floppy = True

  if args.floppy:
    ALIGNMENT = 512

  if args.action == 'create':
    archive = collect(args.files)
    for entry in archive:
      print(entry)
    save(args.image, args.floppy, archive, args.bootcode)
  elif args.action == 'list':
    archive = load(args.image, args.floppy)
    for entry in archive:
      print(entry)
  elif args.action == 'extract':
    archive = load(args.image, args.floppy)
    extract(archive, args.files, args.force)
