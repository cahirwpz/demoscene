#!/usr/bin/env python3

from .iff import IffFile
import struct

from collections import namedtuple
from io import StringIO


def UnRLE(bytes_in):
  bytes_in = bytearray(bytes_in)
  bytes_out = StringIO()

  while bytes_in:
    cmd = bytes_in.pop(0)

    if cmd <= 127:
      l = cmd + 1
      s = bytes_in[:l]
      bytes_in = bytes_in[l:]
      bytes_out.write(s)
    else:
      l = 257 - cmd
      s = bytes_in.pop(0)
      bytes_out.write(chr(s) * l)

  out = bytes_out.getvalue()
  bytes_out.close()

  return out


def Deinterleave(data, width, height, depth):
  out = StringIO()
  bytesPerRow = ((width + 15) & ~15) / 8

  for i in range(depth):
    s = bytesPerRow * i
    for j in range(height):
      out.write(data[s:s + bytesPerRow])
      s += bytesPerRow * depth

  return out.getvalue()


BitMapHeader = namedtuple('BitMapHeader', (
  'w', 'h', 'x', 'y', 'nPlanes', 'masking', 'compression', 'transparentColor',
  'xAspect', 'yAspect', 'pageWidth', 'pageHeight'))
Color = namedtuple('Color', 'r g b')
ColorRange = namedtuple('ColorRange', 'rate flags low high')
PaletteChanges = namedtuple('PaletteChanges', (
  'compression', 'flags', 'startLine', 'lineCount', 'changedLines',
  'minReg', 'maxReg', 'maxChanges', 'totalChanges'))
LineChanges = namedtuple('LineChanges', ('line', 'colorChange'))


class ILBM(IffFile):
  def __init__(self):
    super(ILBM, self).__init__('ILBM')

  def readBMHD(self, data):
    return BitMapHeader(*struct.unpack('>HHhhBBBxHBBhh', data.read(20)))

  def writeBMHD(self, data, out):
    out.write(struct.pack('>HHhhBBBxHBBhh', *data))

  def readANNO(self, data):
    return data.getvalue().rstrip('\x00 ')

  def readPCHG(self, data):
    pchg = PaletteChanges(*struct.unpack('>HHhHHHHHI', data.read(20)))
    assert pchg.flags == 1, "Only PCHG_RGB12 supported!"
    maskLength = ((pchg.lineCount + 31) & ~31) / 8
    lines = []
    changedLines = data.read(maskLength)
    for line in range(pchg.lineCount):
      mask = 1 << (7 - (line & 7))
      if ord(changedLines[line / 8]) & mask:
        count = struct.unpack('>Bx', data.read(2))[0]
        changes = [data.read(2) for i in range(count)]
        lines.append(LineChanges(line + pchg.startLine, changes))
    return (pchg, lines)

  def readCAMG(self, data):
    return struct.unpack('>I', data.read(4))[0]

  def writeCAMG(self, data, out):
    out.write(struct.pack('>I', data))

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
    for r, g, b in data:
      out.write(struct.pack('>BBB', r, g, b))

  def readBODY(self, data):
    return data

  def writeBODY(self, data, out):
    out.write(data)
