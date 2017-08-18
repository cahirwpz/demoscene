#!/usr/bin/env python

from __future__ import print_function

import Image
import sys
from array import array
from pprint import pprint
from collections import namedtuple


def main():
    img = Image.open(sys.argv[1])
    width, height = img.size
    pix = img.load()
    pal = img.getpalette()

    # {16x16 tile as hex string : tile number}
    tiles = dict()
    ntiles = 0

    with open('tiles.dat', 'w') as f:
        print('@map %d %d' % (width / 16, height / 16), file=f)

        for y in range(0, height, 16):
            row = []
            for x in range(0, width, 16):
                tile = array('B')
                for j in range(16):
                    for i in range(16):
                        tile.append(pix[x+i,y+j])
                tile = tile.tostring().encode('hex')
                if tile not in tiles:
                    tiles[tile] = ntiles
                    ntiles += 1
                print(tiles[tile], end=' ', file=f)
            print('', file=f)

        print('@end', file=f)

    # [(tile number, 16x16 tile as hex string), ...]
    tiles = [(i, t) for t, i in tiles.items()]
    tiles.sort()

    img = Image.new('P', (16, len(tiles) * 16))
    pix = img.load()
    for i, t in tiles:
        tile = array('B')
        tile.fromstring(t.decode('hex'))
        for y in range(16):
            for x in range(16):
                pix[x,y + i * 16] = tile[x + y * 16]

    img.putpalette(pal)
    img.save('tiles.png', 'PNG')


if __name__ == '__main__':
    main()
