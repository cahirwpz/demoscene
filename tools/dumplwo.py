#!/usr/bin/env python3

import argparse
import binascii
import io
import logging
import os
import struct
from chunk import Chunk
from collections import namedtuple
from collections.abc import Sequence
from pprint import pprint


class IffData(io.BytesIO):

    def eof(self):
        return self.tell() >= len(self.getvalue())

    def __repr__(self):
        return "<<< binary data of %d bytes >>>" % len(self.getvalue())


class IffChunk(object):
    __slots__ = ('name', 'data')

    def __init__(self, name, data):
        self.name = name
        self.data = data


class IffFile(Sequence):
    ChunkAliasMap = {}
    ChunkBlackList = []

    @classmethod
    def fromFile(cls, filename):
        iff = cls()
        if iff.load(filename):
            return iff

    def __init__(self, form):
        self.form = form
        self.chunks = []

    def load(self, filename):
        self.chunks = []

        with open(filename, 'rb') as iff:
            chunk = Chunk(iff)

            logging.info('Reading file "%s" as IFF/%s type.' %
                         (filename, self.form))

            if chunk.getname().decode() == 'FORM' and (
                    chunk.read(4).decode() == self.form):
                iff.seek(12)

                while True:
                    try:
                        chunk = Chunk(iff)
                    except EOFError:
                        break

                    name = chunk.getname().decode()
                    size = chunk.getsize()
                    data = chunk.read()

                    if name in self.ChunkBlackList:
                        logging.info('Ignoring %s chunk of size %d' %
                                     (name, size))
                    else:
                        logging.debug(
                            'Encountered %s chunk of size %d' % (name, size))

                        self.chunks.append(self.readChunk(name, data))
            else:
                logging.warning(
                    'File %s is not of IFF/%s type.' % (filename, self.form))
                return False

        return True

    def readChunk(self, name, data):
        orig_name = name

        for alias, names in self.ChunkAliasMap.items():
            if name in names:
                name = alias

        handler = getattr(self, 'read%s' % name, None)
        arg = IffData(data)

        if handler:
            data = handler(arg)
        else:
            data = binascii.hexlify(arg.getvalue())
            logging.warning('No handler for %s chunk.' % orig_name)

        return IffChunk(orig_name, data)

    def get(self, name, always_list=False):
        chunks = [c for c in self.chunks if c.name == name]

        if not chunks and not always_list:
            raise ValueError('No chunk named %s.' % name)

        if len(chunks) == 1 and not always_list:
            return chunks[0]
        else:
            return chunks

    def __getitem__(self, name):
        return self.get(name)

    def __iter__(self):
        return iter(self.chunks)

    def __len__(self):
        return len(self.chunks)


Vertex = namedtuple('Vertex', 'x y z')
Color = namedtuple('Color', 'r g b')
Polygon = namedtuple('Polygon', 'points surface')
Surface = namedtuple('Surface', 'name color sideness')


class LWOParserMixin(object):

    def parseMiniChunks(self, string):
        data = IffData(string)
        chunks = []

        while not data.eof():
            name = data.read(4).decode()
            size = self.readInt16(data)
            chunk = data.read(size)
            logging.debug('Encountered %s subchunk of size %d' % (name, size))
            chunks.append(self.readChunk(name, chunk))

        return dict((c.name, c.data) for c in chunks)

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
        byteData = data.getvalue()
        end = byteData.index(b'\0', begin) + 1
        if end & 1:
            end += 1
        return data.read(end - begin).decode("utf-8").rstrip('\0')

    def readVertex(self, data):
        return Vertex(*struct.unpack('>fff', data.read(12)))

    def readPNTS(self, data):
        points = []

        while not data.eof():
            points.append(self.readVertex(data))

        return points

    def readCLIP(self, data):
        index = self.readInt32(data)
        return index, self.parseMiniChunks(data.read())

    @property
    def points(self):
        return self.get('PNTS')


class LWO2(IffFile, LWOParserMixin):
    ChunkAliasMap = {
        'Int16': ['AXIS', 'CSYS', 'ENAB', 'IMAG', 'NEGA', 'NSTA', 'PIXB',
                  'PROJ', 'SIDE'],
        'Int32': ['FLAG', 'VERS'],
        'FloatWithEnvelope': ['ALPH', 'DIFF', 'GLOS', 'LUMI', 'SPEC', 'WRPH',
                              'WRPW', 'TRAN', 'TRNL', 'BUMP', 'RIND', 'REFL',
                              'TAMP'],
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
        result = self.parseMiniChunks(data.read())
        self._blok = False
        return result

    def readCHAN(self, data):
        return data.read(4)

    def readCNTR(self, data):
        return [self.readVertex(data), self.readIndex(data)]

    def readFALL(self, data):
        return [self.readInt16(data), self.readVertex(data),
                self.readIndex(data)]

    def readLAYR(self, data):
        return [self.readInt16(data), self.readInt16(data),
                self.readVertex(data), self.readString(data)]

    def readIMAP(self, data):
        return [self.readString(data), self.parseMiniChunks(data.read())]

    def readOPAC(self, data):
        return [self.readInt16(data), self.readFloat(data),
                self.readIndex(data)]

    def readPOLS(self, data):
        polyType = data.read(4)
        polygons = []

        while not data.eof():
            points = struct.unpack('>H', data.read(2))[0]
            indices = struct.unpack('>' + 'H' * points, data.read(2 * points))
            polygons.append(indices)

        return (polyType, polygons)

    def readNLOC(self, data):
        return [self.readFloat(data), self.readFloat(data)]

    def readNODS(self, data):
        name = data.read(4)
        size = self.readInt16(data)
        return [name, self.parseMiniChunks(data.read(size))]

    def readPTAG(self, data):
        tagType = data.read(4).decode()

        assert tagType in ['SURF', 'COLR', 'PART']

        tags = []

        while not data.eof():
            tags.append(struct.unpack('>HH', data.read(4)))

        return (tagType, tags)

    def readSURF(self, data):
        name = self.readString(data)
        source = self.readString(data)
        chunks = self.parseMiniChunks(data.read())
        return (name, source, chunks)

    def readTAGS(self, data):
        tags = []

        while not data.eof():
            tags.append(self.readString(data))

        return tags

    def readTMAP(self, data):
        return self.parseMiniChunks(data.read())

    def readVMPA(self, data):
        return [self.readInt32(data), self.readColor(data)]

    def readVMAP(self, data):
        if self._blok:
            return self.readString(data)

        tag = data.read(4)
        dimensions = self.readInt16(data)
        name = self.readString(data)
        mapping = []

        while not data.eof():
            vert = self.readIndex(data)
            values = [self.readFloat(data) for i in range(dimensions)]
            mapping.append((vert, values))

        return (tag, dimensions, name, mapping)

    def readVMAD(self, data):
        tag = data.read(4)
        dimensions = self.readInt16(data)
        name = self.readString(data)
        mapping = []

        while not data.eof():
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


class LWOB(IffFile, LWOParserMixin):
    """ http://sandbox.de/osg/lightwave.htm """

    ChunkAliasMap = {
        'Int16': ['FLAG', 'DIFF', 'LUMI', 'SPEC', 'GLOS', 'TFLG', 'REFL',
                  'TRAN', 'TVAL'],
        'Float': ['VDIF', 'SMAN', 'EDGE', 'TAAS', 'TAMP', 'TFP0', 'RIND',
                  'VSPC', 'VLUM', 'TOPC', 'VTRN'],
        'Color': ['COLR', 'TCLR'],
        'Vertex': ['TSIZ', 'TCTR', 'TFAL', 'TVEL'],
        'String': ['TIMG', 'BTEX', 'CTEX', 'DTEX', 'LTEX', 'TTEX']}

    def __init__(self):
        super(LWOB, self).__init__('LWOB')

    def readPOLS(self, data):
        polygons = []

        while not data.eof():
            points = struct.unpack('>H', data.read(2))[0]
            pointdata = data.read((points + 1) * 2)

            words = struct.unpack('>' + 'H' * (points + 1), pointdata)
            polygons.append([words[:-1], words[-1]])

        return polygons

    def readSRFS(self, data):
        return [name for name in data.read().split('\0') if name]

    def readSURF(self, data):
        name = self.readString(data)
        return (name, self.parseMiniChunks(data.read()))

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


def convertLWO2(lwo, output):
    def out(s):
        string_as_bytes = "{0}\n".format(s).encode()
        output.write(string_as_bytes)

    tags = list(lwo['TAGS'].data)
    clips = lwo.get('CLIP', always_list=True)

    if clips:
        out('@image.cnt %d\n' % len(clips))

    for clip in clips:
        index, imag = clip.data
        out('@image %d' % index)
        out('file "%s"' % imag['STIL'])
        out('@end\n')

    txuv = {}
    for vmap in lwo.get('VMAP', always_list=True):
        if vmap.data[0] == 'TXUV':
            txuv[vmap.data[2]] = vmap.data[3]

    srfs = list(tags)
    for surf in lwo.get('SURF', always_list=True):
        i = tags.index(surf.data[0])
        srfs[i] = (surf.data[0], surf.data[2])

    out('@surf.cnt %d\n' % len(tags))

    surf_vmap = {}
    for surf in srfs:
        if type(surf) is str:
            continue
        name, surf = surf
        out('@surf %d' % tags.index(name))
        for key, value in surf.items():
            if key == 'COLR':
                out('color %d %d %d' % value)
            if key == 'SIDE':
                out('side %d' % value)
            if key == 'BLOK':
                for key, value in value.items():
                    if key == 'IMAG':
                        out('texture %d' % value)
                    if key == 'VMAP':
                        surf_vmap[value] = tags.index(name)
        out('@end\n')

    pnts = lwo['PNTS']
    out('@pnts %d' % len(pnts.data))
    for point in pnts.data:
        out('%f %f %f' % point)
    out('@end\n')

    vmap_txuv = []
    for name, values in txuv.items():
        surface = surf_vmap.get(name, None)
        if not surface:
            continue
        for vertex, uv in values:
            vmap_txuv.append((surface, vertex, tuple(uv)))

    if vmap_txuv:
        out('@pnts.uv %d' % len(vmap_txuv))
        for _, _, uv in vmap_txuv:
            out('%f %f' % uv)
        out('@end\n')

    pols_surf = {}
    for ptag in lwo.get('PTAG', always_list=True):
        if ptag.data[0] == 'SURF':
            ptag = ptag.data[1]
            for polygon, surface in ptag:
                pols_surf[polygon] = surface

    pols = lwo['POLS'].data[1]
    out('@pols %d %d' % (len(pols), sum(map(len, lwo['POLS'].data[1]))))
    for i, polygon in enumerate(pols):
        out('%d %s' % (pols_surf[i], ' '.join(map(str, polygon))))
    out('@end\n')

    if vmap_txuv:
        pols_txuv = {}
        for i, txuv in enumerate(vmap_txuv):
            surface, vertex, _ = txuv
            surf_dict = pols_txuv.get(surface, {})
            surf_dict[vertex] = i
            pols_txuv[surface] = surf_dict

        out('@pols.uv %d %d' % (len(pols), sum(map(len, lwo['POLS'].data[1]))))
        for i, vertices in enumerate(pols):
            surface = pols_surf[i]
            vertices = [pols_txuv[surface][v] for v in vertices]
            out(' '.join(map(str, vertices)))
        out('@end\n')


def main():
    parser = argparse.ArgumentParser(
        description=("Converts Lightwave Object(LWOB/LWO2) "
                     "file to textual representation."))
    parser.add_argument(
        '-q', '--quiet', action='store_true',
        help='Silence out diagnostic messages.')
    parser.add_argument(
        '-f', '--force', action='store_true',
        help='If the output object exists, the tool will' 'overwrite it.')
    parser.add_argument(
        'input', metavar='LWO', type=str,
        help='Input LightWave object file name.')
    parser.add_argument(
        'output', metavar='OBJ', type=str, nargs='?',
        help='Output object file name.')
    args = parser.parse_args()

    args.input = os.path.abspath(args.input)

    logLevel = [logging.INFO, logging.WARNING][args.quiet]
    logging.basicConfig(level=logLevel, format='%(levelname)s: %(message)s')

    if not os.path.isfile(args.input):
        raise SystemExit('Input file "%s" does not exists!' % args.input)

    if args.output:
        name, _ = os.path.splitext(args.output)
        args.output = os.path.join(name + '.3d')

        if os.path.exists(args.output) and not args.force:
            raise SystemExit(
                'Object file "%s" already exists '
                '(use "-f" to override).' % args.output)

    lwo = LWO2.fromFile(args.input)
    if not lwo:
        lwo = LWOB.fromFile(args.input)
    if not lwo:
        raise SystemExit('File format not recognized.')

    if not args.output:
        for chunk in lwo.chunks:
            pprint((chunk.name, chunk.data))

    if args.output and lwo.form == 'LWO2':
        logging.info('Writing object structure to %s file.' % args.output)
        with open(str(args.output), 'wb') as f:
            convertLWO2(lwo, f)


if __name__ == '__main__':
    main()
