#!/usr/bin/env python

import argparse
import logging
import os
import struct

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
    return [self.handlePoint(data[i:i+12]) for i in xrange(0, len(data), 12)]

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

  @property
  def polygons(self):
    for name, chunk in self._chunks:
      if name == 'POLS':
        return chunk
        
  @property
  def points(self):
    for name, chunk in self._chunks:
      if name == 'PNTS':
        return chunk


def main():
  parser = argparse.ArgumentParser(
      description=('Converts Lightwave Object (LWOB) file to raw data.'))
  parser.add_argument('-f', '--force', action='store_true',
      help='If output object exists, the tool will overwrite it.')
  parser.add_argument('input', metavar='LWOB', type=str,
      help='Input LightWave object (LWOB) file name.')
  parser.add_argument('output', metavar='RAWOBJ', type=str,
      help='Output Raw Object file name.')
  args = parser.parse_args()

  args.input = os.path.abspath(args.input)

  if not args.output.endswith('.robj'):
    try:
      name = args.input.rsplit('.', 1)[0]
    except:
      pass
    args.output = os.path.join(name + '.robj')

  if not os.path.isfile(args.input):
    raise SystemExit('Input file "%s" does not exists!' % args.input)

  if os.path.exists(args.output) and not args.force:
    raise SystemExit('Raw Object file "%s" already exists (use'
        ' "-f" to override).' % args.output)

  lwob = LWOB()
  lwob.loadFile(args.input)

  logging.info('Object has %d points, and %d polygons.',
      len(lwob.points), len(lwob.polygons))

  with open(args.output, 'w') as robj:
    robj.write(struct.pack('>HH', len(lwob.points), len(lwob.polygons)))

    for x, y, z in lwob.points:
      robj.write(struct.pack('>fff', x, y, z))

    for points, _ in lwob.polygons:
      assert len(points) == 3
      robj.write(struct.pack('>HHH', *points))

  logging.info('Wrote Raw Object file to: "%s"', args.output)


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')
  main()
