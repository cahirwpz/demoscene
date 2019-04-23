#!/usr/bin/env python3

import os.path
import struct
import argparse
import logging

from PIL import Image
from array import array
from collections import Mapping


PSF1_MAGIC = '\x36\x04'
PSF1_MODE512 = 0x01
PSF1_MODEHASTAB = 0x02
PSF1_MODEHASSEQ = 0x04
PSF1_SEPARATOR = '\uFFFF'
PSF1_STARTSEQ = '\uFFFE'

PSF2_MAGIC = '\x72\xb5\x4a\x86'
PSF2_HAS_UNICODE_TABLE = 0x01
PSF2_SEPARATOR = '\xFF'
PSF2_STARTSEQ = '\xFE'


class PSF(Mapping):
  def __init__(self):
    self.height = 0
    self._char = []
    self._char_map = {}

  def __getitem__(self, key):
    return self._char[self._char_map[key]]

  def __iter__(self):
    return self._char_map.keys()

  def __len__(self):
    return len(self._char_map)

  def readPSF1(self, f):
    mode, self.height = struct.unpack('BB', f.read(2))

    if mode & PSF1_MODE512:
      glyphs = 512
    else:
      glyphs = 256

    self._char = [array('B', f.read(self.height)) for i in range(glyphs)]

    if mode & PSF1_MODEHASTAB:
      index = 0

      while True:
        uchar = f.read(2).decode('utf-16')
        if uchar == '':
          break
        if uchar == PSF1_SEPARATOR:
          index += 1
        else:
          self._char_map[uchar] = index

  def readPSF2(self, f):
    (flags, glyphs, self.height) = struct.unpack('8xIII8x', f.read(28))

    self._char = [array('B', f.read(self.height)) for i in range(glyphs)]

    if flags & PSF2_HAS_UNICODE_TABLE:
      index = 0
      ustr = ''

      while True:
        uchar = f.read(1)
        if uchar == '':
          break
        if uchar == PSF2_SEPARATOR:
          for uchar in ustr:
            self._char_map[uchar] = index
          index += 1
          ustr = ''
        else:
          ustr += uchar

  def fromFile(self, path):
    with open(path, 'rb') as f:
      if f.read(2).decode("utf-8") == PSF1_MAGIC:
        return self.readPSF1(f)
      logging.info("Data doesn't start with PSF1 magic prefix.")

    with open(path, 'rb') as f:
      if f.read(4).decode("utf-8") == PSF2_MAGIC:
        return self.readPSF2(f)
      logging.info("Data doesn't start with PSF2 magic prefix.")

    raise SystemExit('"%s" is not PC Screen Font file!')


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  parser = argparse.ArgumentParser(
    description='Convert PSF font file to PNG image.')
  parser.add_argument('input', metavar='INPUT', type=str,
                      help='PC Screen Font file.')
  args = parser.parse_args()

  if not os.path.isfile(args.input):
    raise SystemExit('Input file does not exists!')

  output = os.path.splitext(args.input)[0] + '.png'

  psf = PSF()
  psf.fromFile(args.input)

  last = 127

  font = Image.new('L', (psf.height, (last - 33) * 8))
  im = font.load()

  for i in range(last - 33):
    uchar = struct.pack('B', i + 33).decode('latin2')
    data = psf.get(uchar, None)
    if data:
      for j in range(psf.height):
        for k in range(8):
          im[k, i * 8 + j] = 255 if data[j] & (1 << (7 - k)) else 0

  font.save(output)

# vim:expandtab ts=2 sw=2:
