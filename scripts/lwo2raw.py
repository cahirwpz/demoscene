#!/usr/bin/env python

import logging
import sys
import struct
import pprint

from collections import namedtuple

from util import iff

Point = namedtuple('Point', 'x y z')
Color = namedtuple('Color', 'r g b')
Polygon = namedtuple('Polygon', 'points surface')


class LWOB(iff.Parser):
  ChunkAliasMap = {
      'Int16': ['FLAG', 'DIFF', 'LUMI', 'SPEC', 'GLOS', 'TFLG', 'REFL', 'TRAN', 'TVAL'],
      'Float': ['VDIF', 'SMAN', 'EDGE', 'TAAS', 'TAMP', 'TFP0', 'RIND', 'VSPC', 'VLUM', 'TOPC', 'VTRN'],
      'Color': ['COLR', 'TCLR'],
      'Point': ['TSIZ', 'TCTR', 'TFAL', 'TVEL'],
      'String': ['TIMG', 'BTEX', 'CTEX', 'DTEX', 'LTEX', 'TTEX']}

  def __init__(self):
    super(LWOB, self).__init__('LWOB')

  def _parseMiniChunks(self, data):
    chunks = []

    while data:
      name = data[:4]
      size = struct.unpack('>H', data[4:6])[0]
      chunk = data[6:6+size]

      chunks.append(self._parseChunk(name, chunk))

      data = data[6+size:]

    return chunks

  def handlePNTS(self, data):
    return [Point(*struct.unpack('>fff', data[i:i+12]))
            for i in xrange(0, len(data), 12)]

  def handlePOLS(self, data):
    polygons = []

    while data:
      points = struct.unpack('>H', data[:2])[0]
      step = (points+2)*2

      words = struct.unpack('>' + 'H' * (points+1), data[2:step])
      polygons.append(Polygon(words[:-1], words[-1]))

      data = data[step:]
    
    return polygons

  def handleSRFS(self, data):
    return [name for name in data.split('\0') if name]

  def handleSURF(self, data):
    i = data.index('\0')
    name = data[:i]
    data = data[i:].lstrip('\0')

    return (name, self._parseMiniChunks(data))

  def handleFloat(self, data):
    return struct.unpack('>f', data)[0]

  def handleInt16(self, data):
    return struct.unpack('>H', data)[0]

  def handleColor(self, data):
    return Color(*struct.unpack('>BBBx', data))

  def handlePoint(self, data):
    return Point(*struct.unpack('>fff', data))

  def handleString(self, data):
    return data[:data.index('\0')]

  def handleTWRP(self, data):
    return struct.unpack('>HH', data)


def main(argv):
  lwob = LWOB()
  lwob.loadFile(sys.argv[1])

  pprint.pprint(lwob._chunks)

if __name__ == '__main__':
  FORMAT = '%(levelname)s: %(message)s'
  logging.basicConfig(format=FORMAT, level=logging.DEBUG)
  main(sys.argv)
