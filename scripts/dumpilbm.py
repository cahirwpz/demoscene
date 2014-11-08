#!/usr/bin/env python

import logging
import struct
import sys

from util import iff
from pprint import pprint
from collections import namedtuple

BitMapHeader = namedtuple('BitMapHeader', (
  'w', 'h', 'x', 'y', 'nPlanes', 'masking', 'compression', 'transparentColor',
  'xAspect', 'yAspect', 'pageWidth', 'pageHeight'))
Color = namedtuple('Color', 'r g b')
ColorRange = namedtuple('ColorRange', 'rate flags low high')


class ILBM(iff.Parser):
  def __init__(self):
    super(ILBM, self).__init__('ILBM')

  def readBMHD(self, data):
    return BitMapHeader(*struct.unpack('>HHhhBBBxHBBhh', data.read(20)))

  def readCRNG(self, data):
    return ColorRange(*struct.unpack('>xxhhBB', data.read(8)))

  def readCMAP(self, data):
    cmap = []
    try:
      while True:
        cmap.append(Color(*struct.unpack('>BBB', data.read(3))))
    except struct.error:
      pass
    return cmap

  def readBODY(self, data):
    return data


def main():
  ilbm = ILBM()

  if ilbm.load(sys.argv[1]):
    for chunk in ilbm:
      pprint(chunk)


if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
  main()
