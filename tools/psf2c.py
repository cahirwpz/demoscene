#!/usr/bin/env python3

import os.path
import struct
import argparse
import logging

from array import array
from collections.abc import Mapping


class Font(Mapping):

    def __init__(self, height, char, char_map):
        self.height = height
        self._char = char
        self._char_map = char_map

    def __getitem__(self, key):
        return self._char[self._char_map[key]]

    def __iter__(self):
        return iter(self._char_map.keys())

    def __len__(self):
        return len(self._char_map)


PSF1_MAGIC = b'\x36\x04'
PSF1_MODE512 = 0x01
PSF1_MODEHASTAB = 0x02
PSF1_MODEHASSEQ = 0x04
PSF1_SEPARATOR = '\uFFFF'
PSF1_STARTSEQ = '\uFFFE'


def readPSF1(f):
    mode, height = struct.unpack('BB', f.read(2))

    if mode & PSF1_MODE512:
        glyphs = 512
    else:
        glyphs = 256

    char = [array('B', f.read(height)) for i in range(glyphs)]
    char_map = {}

    assert mode & PSF1_MODEHASTAB

    index = 0

    while True:
        uchar = f.read(2).decode('utf-16')
        if uchar == '':
            break
        if uchar == PSF1_SEPARATOR:
            index += 1
        else:
            char_map[uchar] = index

    return Font(height, char, char_map)


PSF2_MAGIC = b'\x72\xb5\x4a\x86'
PSF2_HAS_UNICODE_TABLE = 0x01
PSF2_SEPARATOR = b'\xFF'
PSF2_STARTSEQ = b'\xFE'


def readPSF2(f):
    flags, glyphs, height = struct.unpack('8xIII8x', f.read(28))

    char = [array('B', f.read(height)) for i in range(glyphs)]
    char_map = {}

    assert flags & PSF2_HAS_UNICODE_TABLE

    index = 0
    ustr = b''

    while True:
        uchar = f.read(1)
        if uchar == b'':
            break
        if uchar == PSF2_SEPARATOR:
            for uchar in ustr:
                char_map[chr(uchar)] = index
            index += 1
            ustr = b''
        else:
            ustr += uchar

    return Font(height, char, char_map)


def readPSF(path):
    with open(path, 'rb') as f:
        if f.read(2) == PSF1_MAGIC:
            return readPSF1(f)

    with open(path, 'rb') as f:
        if f.read(4) == PSF2_MAGIC:
            return readPSF2(f)

    raise SystemExit('"%s" is not PC Screen Font file!' % path)


def convert(path, name):
    font = readPSF(path)

    print('static __data_chip u_short _%s_glyphs[] = {' % name)
    for i in range(33, 127):
        uchar = struct.pack('B', i).decode('latin2')
        data = font.get(uchar, None)
        if data:
            print('  /* char: %s */' % uchar)
            for byte in data:
                print('  0x%04x,' % (byte << 8))
    print('};')
    print('')

    print('static BitmapT _%s_bm = {' % name)
    print('  .width = 16,')
    print('  .height = %d,' % font.height)
    print('  .depth = 1,')
    print('  .bytesPerRow = 2,')
    print('  .bplSize = %d,' % (2 * font.height // 8))
    print('  .flags = BM_DISPLAYABLE|BM_STATIC,')
    print('  .planes = {')
    print('    _%s_glyphs' % name)
    print('  }')
    print('};')
    print('')

    print('FontT %s = {' % name)
    print('  .data = &_%s_bm,' % name)
    print('  .height = %d,' % font.height)
    print('  .space = 8,')
    print('  .charmap = {')
    y = 0
    for i in range(33, 127):
        uchar = struct.pack('B', i).decode('latin2')
        data = font.get(uchar, None)
        print('    {.y = %3d, .width = 8}, /* char: %s */' % (y, uchar))
        if data:
            y += font.height
    print('  }')
    print('};')


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG,
                        format='%(levelname)s: %(message)s')

    parser = argparse.ArgumentParser(
        description='Convert PSF font file to C representation.')
    parser.add_argument('--name', metavar='NAME', type=str,
                        help='Base name of C objects.')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='PC Screen Font file.')
    args = parser.parse_args()

    if not os.path.isfile(args.path):
        raise SystemExit('Input file does not exists!')

    convert(args.path, args.name)
