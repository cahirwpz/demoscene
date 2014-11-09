#!/usr/bin/env python

import iff
import struct

from collections import namedtuple

BitMapHeader = namedtuple('BitMapHeader', (
  'w', 'h', 'x', 'y', 'nPlanes', 'masking', 'compression', 'transparentColor',
  'xAspect', 'yAspect', 'pageWidth', 'pageHeight'))
Color = namedtuple('Color', 'r g b')
ColorRange = namedtuple('ColorRange', 'rate flags low high')


class ILBM(iff.File):
  def __init__(self):
    super(ILBM, self).__init__('ILBM')

  def readBMHD(self, data):
    return BitMapHeader(*struct.unpack('>HHhhBBBxHBBhh', data.read(20)))

  def writeBMHD(self, data, out):
    out.write(struct.pack('>HHhhBBBxHBBhh', *data))

  def readCAMG(self, data):
    return struct.unpack('>H', data.read(2))[0]

  def writeCAMG(self, data, out):
    out.write(struct.pack('>H', data))

  def readCRNG(self, data):
    return ColorRange(*struct.unpack('>xxhhBB', data.read(8)))

  def writeCRNG(self, data, out):
    out.write(struct.pack('>xxhhBB', *data))

  def readCMAP(self, data):
    cmap = []
    try:
      while True:
        cmap.append(Color(*struct.unpack('>BBB', data.read(3))))
    except struct.error:
      pass
    return cmap

  def writeCMAP(self, data, out):
    for color in data:
      out.write(struct.pack('>BBB', *color))

  def readBODY(self, data):
    return data

  def writeBODY(self, data, out):
    out.write(data)
