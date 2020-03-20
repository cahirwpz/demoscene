#!/usr/bin/env python3

from array import array
import argparse
import base64
import os.path
import xml.etree.ElementTree as ET
import zlib
from PIL import Image


class TileSet(object):
    def __init__(self, name, width, height, tiles):
        self.name = name
        self.width = width
        self.height = height
        self.tiles = tiles

    def get(self, i):
        return self.tiles[i]

    def optimize(self, unique_tiles):
        self.tiles = [self.tiles[tile] for tile in unique_tiles]

    def save(self, name):
        im = Image.new('P', (self.width, self.height * len(self.tiles)))
        im.putpalette(self.tiles[0].getpalette())
        for i, tile in enumerate(self.tiles):
            im.paste(tile, box=(0, i * self.height))
        im.save(name)


def parseTileSet(node, workdir):
    name = node.attrib['name']
    spacing = int(node.attrib['spacing'])
    tilecount = int(node.attrib['tilecount'])
    columns = int(node.attrib['columns'])
    tilewidth = int(node.attrib['tilewidth'])
    tileheight = int(node.attrib['tileheight'])
    image = node[0]
    source = image.attrib['source']

    im = Image.open(os.path.join(workdir, source))

    tiles = []
    w, h = tilewidth, tileheight

    y = 0
    for row in range(tilecount // columns):
        x = 0
        for col in range(columns):
            tiles.append(im.crop(box=(x, y, x + w, y + h)))
            x += w + spacing
        y += h + spacing

    return TileSet(name, w, h, tiles)


class Layer(object):
    def __init__(self, name, width, height, tiles):
        self.name = name
        self.width = width
        self.height = height
        self.tiles = [i - 1 for i in tiles]

    def get(self, x, y):
        assert(0 <= x < self.width)
        assert(0 <= y < self.height)
        return self.tiles[x + y * self.width]

    def optimize(self, unique_tiles):
        renum = {old: new for new, old in enumerate(unique_tiles)}
        self.tiles = list(map(lambda x: renum[x], self.tiles))

    def save(self, name):
        tab = array('H', self.tiles)
        tab.byteswap()
        with open(name, 'wb') as f:
            f.write(tab.tobytes())


def parseLayer(node):
    name = node.attrib['name']
    width = int(node.attrib['width'])
    height = int(node.attrib['height'])
    data = node[0].text
    tiles = array('I')
    tiles.frombytes(zlib.decompress(base64.b64decode(data),
                                    16 + zlib.MAX_WBITS))
    return Layer(name, width, height, tiles)


def preview(name, layer, tileset):
    tw, th = tileset.width, tileset.height
    im = Image.new('RGB', (layer.width * tw, layer.height * th))
    for y in range(layer.height):
        for x in range(layer.width):
            tile = tileset.get(layer.get(x, y))
            im.paste(tile, box=(x * tw, y * th))
    im.save(name + '.png')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Convert Tiled map to internal format.')
    parser.add_argument(
        'input', metavar='INPUT', type=str,
        help='Tiled map containing layer and tileset.')

    args = parser.parse_args()
    basename = os.path.splitext(args.input)[0]
    workdir = os.path.dirname(args.input)

    layer = None
    tileset = None

    tree = ET.parse(args.input)
    root = tree.getroot()

    for child in root:
        # print(child.tag, child.attrib)
        if child.tag == 'layer':
            layer = parseLayer(child)
        if child.tag == 'tileset':
            tileset = parseTileSet(child, workdir)

    unique_tiles = list(sorted(set(layer.tiles)))

    layer.optimize(unique_tiles)
    layer.save(basename + '-map.bin')
    tileset.optimize(unique_tiles)
    tileset.save(basename + '-tiles.png')

    with open(basename + '.h', 'w') as f:
        print('static TileSetT %s = TILESET(%d, %d, %d, "%s");' %
              (tileset.name, tileset.width, tileset.height,
               len(tileset.tiles), basename + '-tiles.ilbm'))
        print('static TileMapT %s = TILEMAP(%d, %d, "%s");' %
              (layer.name, layer.width, layer.height, basename + '-map.bin'))
