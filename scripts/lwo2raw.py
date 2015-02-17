#!/usr/bin/env python

import argparse
import json
import logging
import os
import struct

from array import array
from collections import namedtuple
from pprint import pprint

from util import iff

Vertex = namedtuple('Vertex', 'x y z')
Color = namedtuple('Color', 'r g b')
Polygon = namedtuple('Polygon', 'points surface')
Surface = namedtuple('Surface', 'name color sideness')


class LWOParserMixin(object):
  def _parseMiniChunks(self, string):
    data = iff.StringIO(string)
    chunks = []

    while data.tell() < len(data):
      name = data.read(4)
      size = self.readInt16(data)
      chunk = data.read(size)
      logging.debug('Encountered %s subchunk of size %d' % (name, size))
      chunks.append(self._readChunk(name, chunk))

    return chunks

  def readColor(self, data):
    return Color(*struct.unpack('>BBBx', data.read(4)))

  def readColor12(self, data):
    r, g, b = struct.unpack('>fffH', data.read(14))[:3]
    return Color(int(r * 255), int(g * 255), int(b * 255))

  def readFloat(self, data):
    return struct.unpack('>f', data.read(4))[0]

  def readIndex(self, data):
    vx = self.readInt16(data)
    if vx & 0xff00 == 0xff00:
      return ((vx & 0xff) << 16) | self.readInt16(data)
    else:
      return vx

  def readInt16(self, data):
    return struct.unpack('>H', data.read(2))[0]

  def readInt32(self, data):
    return struct.unpack('>I', data.read(4))[0]

  def readString(self, data):
    begin = data.tell()
    string = data.getvalue()
    end = string.index('\0', begin) + 1
    if end & 1:
      end += 1
    return data.read(end - begin).rstrip('\0')

  def readVertex(self, data):
    return Vertex(*struct.unpack('>fff', data.read(12)))

  def readPNTS(self, data):
    points = []

    while data.tell() < len(data):
      points.append(self.readVertex(data))

    return points

  def readCLIP(self, data):
    index = self.readInt32(data)
    return (index, self._parseMiniChunks(data.read()))

  @property
  def points(self):
    return self.get('PNTS')


class LWO2(iff.File, LWOParserMixin):
  ChunkAliasMap = {
    'Int16': ['AXIS', 'CSYS', 'ENAB', 'IMAG', 'NEGA', 'NSTA', 'PIXB', 'PROJ',
              'SIDE'],
    'Int32': ['FLAG', 'VERS'],
    'FloatWithEnvelope': ['DIFF', 'LUMI', 'SPEC', 'WRPH', 'WRPW'],
    'Float': ['NZOM', 'SMAN'],
    'String': ['OREF', 'STIL'],
    'Color12': ['COLR'],
    'CNTR': ['ROTA', 'SIZE']}

  def __init__(self):
    super(LWO2, self).__init__('LWO2')

    self._blok = False

  def readFloatWithEnvelope(self, data):
    return [self.readFloat(data), self.readIndex(data)]

  def readAAST(self, data):
    return [self.readInt16(data), self.readFloat(data)]

  def readBBOX(self, data):
    return [self.readVertex(data), self.readVertex(data)]

  def readBLOK(self, data):
    self._blok = True
    result = self._parseMiniChunks(data.read())
    self._blok = False
    return result

  def readCHAN(self, data):
    return data.read(4)

  def readCNTR(self, data):
    return [self.readVertex(data), self.readIndex(data)]

  def readFALL(self, data):
    return [self.readInt16(data), self.readVertex(data), self.readIndex(data)]

  def readLAYR(self, data):
    return [self.readInt16(data), self.readInt16(data), self.readVertex(data),
            self.readString(data)]

  def readIMAP(self, data):
    return [self.readString(data), self._parseMiniChunks(data.read())]

  def readOPAC(self, data):
    return [self.readInt16(data), self.readFloat(data), self.readIndex(data)]

  def readPOLS(self, data):
    polyType = data.read(4)
    polygons = []

    while data.tell() < len(data):
      points = struct.unpack('>H', data.read(2))[0]
      indices = struct.unpack('>' + 'H' * points, data.read(2 * points))
      polygons.append(indices)

    return [polyType, polygons]

  def readNLOC(self, data):
    return [self.readFloat(data), self.readFloat(data)]

  def readNODS(self, data):
    name = data.read(4)
    size = self.readInt16(data)
    return [name, self._parseMiniChunks(data.read(size))]

  def readPTAG(self, data):
    tagType = data.read(4)

    assert tagType in ['SURF', 'COLR']

    tags = []

    while data.tell() < len(data):
      tags.append(struct.unpack('>HH', data.read(4)))

    return [tagType, tags]

  def readSURF(self, data):
    name = self.readString(data)
    source = self.readString(data)
    return [name, source, self._parseMiniChunks(data.read())]

  def readTAGS(self, data):
    tags = []

    while data.tell() < len(data):
      tags.append(self.readString(data))

    return tags

  def readTMAP(self, data):
    return self._parseMiniChunks(data.read())

  def readVMPA(self, data):
    return [self.readInt32(data), self.readColor(data)]

  def readVMAP(self, data):
    if self._blok:
      return self.readString(data)

    tag = data.read(4)
    dimensions = self.readInt16(data)
    name = self.readString(data)
    mapping = []

    while data.tell() < len(data):
      vert = self.readIndex(data)
      values = [self.readFloat(data) for i in range(dimensions)]
      mapping.append((vert, values))

    return [tag, dimensions, name, mapping]

  def readVMAD(self, data):
    tag = data.read(4)
    dimensions = self.readInt16(data)
    name = self.readString(data)
    mapping = []

    while data.tell() < len(data):
      vert = self.readIndex(data)
      poly = self.readIndex(data)
      values = [self.readFloat(data) for i in range(dimensions)]
      mapping.append((vert, poly, values))

    return [tag, dimensions, name, mapping]

  def readWRAP(self, data):
    return [self.readInt16(data), self.readInt16(data)]

  @property
  def polygons(self):
    polyType, polygons = self.get('POLS')
    return polygons

  @property
  def polygonTags(self):
    return dict(self.get('PTAG', always_list=True))

  @property
  def tags(self):
    return self.get('TAGS')


class LWOB(iff.File, LWOParserMixin):
  # http://sandbox.de/osg/lightwave.htm

  ChunkAliasMap = {
      'Int16': ['FLAG', 'DIFF', 'LUMI', 'SPEC', 'GLOS', 'TFLG', 'REFL', 'TRAN',
                'TVAL'],
      'Float': ['VDIF', 'SMAN', 'EDGE', 'TAAS', 'TAMP', 'TFP0', 'RIND', 'VSPC',
                'VLUM', 'TOPC', 'VTRN'],
      'Color': ['COLR', 'TCLR'],
      'Vertex': ['TSIZ', 'TCTR', 'TFAL', 'TVEL'],
      'String': ['TIMG', 'BTEX', 'CTEX', 'DTEX', 'LTEX', 'TTEX']}

  def __init__(self):
    super(LWOB, self).__init__('LWOB')

  def readPOLS(self, data):
    polygons = []

    while data.tell() < len(data):
      points = struct.unpack('>H', data.read(2))[0]
      pointdata = data.read((points + 1) * 2)

      words = struct.unpack('>' + 'H' * (points + 1), pointdata)
      polygons.append([words[:-1], words[-1]])

    return polygons

  def readSRFS(self, data):
    return [name for name in data.read().split('\0') if name]

  def readSURF(self, data):
    name = self.readString(data)
    return (name, self._parseMiniChunks(data.read()))

  def readTWRP(self, data):
    return struct.unpack('>HH', data.read(4))

  @property
  def surfaceNames(self):
    return self.get('SRFS')

  @property
  def surfaces(self):
    return dict(self.get('SURF', always_list=True))

  @property
  def polygons(self):
    return self.get('POLS')


def WriteRawObject(filename, points, polygons, surfaces):
  logging.info('Object has %d points, %d polygons and %d surfaces.',
               len(points), len(polygons), len(surfaces))

  with open(filename, 'w') as robj:
    robj.write(struct.pack('>HHH', len(points), len(polygons), len(surfaces)))

    for x, y, z in points:
      robj.write(struct.pack('>fff', x, y, z))

    for points, surf in polygons:
      if len(points) != 3:
        raise SystemExit('Polygon triangulation not yet supported.')
      robj.write(struct.pack('>HHHH', surf, *points))

    for surface in surfaces:
      color = surface.color or Color(0, 0, 0)

      data = struct.pack('>?BBB%dsx' % len(surface.name),
                         surface.sideness, color.r, color.g, color.b,
                         surface.name)
      robj.write(data)

  logging.info('Wrote Raw Object file to: "%s"', filename)


def main():
  parser = argparse.ArgumentParser(
    description='Converts Lightwave Object (LWOB/LWO2) file to raw data.')
  parser.add_argument(
    '-f', '--force', action='store_true',
    help='If the output object exists, the tool will' 'overwrite it.')
  parser.add_argument(
    '-c', '--colors', action='store_true',
    help=('If the object has surfaces with colors assigned, the tool will '
          'write out palette in JSON format.'))
  parser.add_argument(
    'input', metavar='LWO', type=str, help='Input LightWave object file name.')
  parser.add_argument(
    'output', metavar='RAWOBJ', type=str, nargs='?',
    help='Output Raw Object file name.')
  args = parser.parse_args()

  args.input = os.path.abspath(args.input)

  if not os.path.isfile(args.input):
    raise SystemExit('Input file "%s" does not exists!' % args.input)

  if args.output:
    if not args.output.endswith('.robj'):
      try:
        name = args.output.rsplit('.', 1)[0]
      except:
        pass
      args.output = os.path.join(name + '.robj')

    if os.path.exists(args.output) and not args.force:
      raise SystemExit(
        'Raw Object file "%s" already exists (use "-f" to override).' %
        args.output)

  lwo = None
  lwob = LWOB()
  lwo2 = LWO2()

  if lwob.load(args.input):
    lwo = lwob
  elif lwo2.load(args.input):
    lwo = lwo2
  else:
    raise SystemExit('File format not recognized.')

  if type(lwo) == LWOB:
    print lwo.polygons
    polygons = [Polygon(points, surf) for points, surf in lwo.polygons.data]
    surfaces = []
    for surface in lwo['SURF']:
      name, props = surface.data
      color = props.get('COLR', None)
      sideness = bool(props.get('FLAG', 0) & 0x0100)
      surfaces.append(Surface(name, color, sideness))

  if type(lwo) == LWO2:
    surfaces = []

    for surface in lwo['SURF']:
      name, props = surface[0], dict(surface[2])
      color = props.get('COLR', None)
      sideness = props.get('SIDE', 1) == 3
      surfaces.append(Surface(name, color, sideness))

    surfenum = {}

    for orig, name in enumerate(lwo['TAGS']):
      for num, surf in enumerate(surfaces):
        if name == surf.name:
          surfenum[orig] = num

    polysurf = [surfenum[num] for _, num in dict(lwo['PTAG'])['SURF']]
    polygons = [Polygon(points, surf)
                for points, surf in zip(lwo.polygons, polysurf)]

  if args.output:
    WriteRawObject(args.output, lwo.points, polygons, surfaces)

    if args.colors:
      colors = [tuple(surface.color) for surface in surfaces if surface.color]

      filename = args.output.rsplit('.', 1)[0] + '.json'
      with open(filename, 'w') as fp:
        json.dump(colors, fp)
        fp.write('\n')
        logging.info('Wrote surface colors to: "%s" file.', filename)

      filename = args.output.rsplit('.', 1)[0] + '.pal'
      with open(filename, 'w') as fp:
        pal = array('B')

        for color in colors:
          pal.extend(color)

        logging.info('Saving palette of %d surfaces to "%s".',
                     len(colors), filename)
        fp.write(struct.pack('>HH', 0, len(colors)))
        fp.write(pal.tostring())
  else:
    pprint(lwo._chunks)


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format='%(levelname)s: %(message)s')
  main()
