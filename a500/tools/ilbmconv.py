#!/usr/bin/env python

import Image
import math
import argparse
from array import array
from iff.iff import IffChunk
from iff.ilbm import ILBM, BitMapHeader, Color


def c2p(pix, width, height, depth):
    data = array('H')
    padding = array('B', [0 for _ in range(16 - (width & 15))])

    for offset in range(0, width * height, width):
        row = pix[offset:offset + width]
        if width & 15:
            row.extend(padding)
        for p in range(depth):
            bits = [(byte >> p) & 1 for byte in row]
            for i in range(0, width, 16):
                word = 0
                for j in range(16):
                    word = word * 2 + bits[i + j]
                data.append(word)

    data.byteswap()
    return data.tostring()


def convert(input, output):
    im = Image.open(input)
    pix = array('B', im.getdata())
    pal = im.getpalette()

    width, height = im.size
    colors = im.getextrema()[1] + 1
    depth = int(math.ceil(math.log(colors, 2)))

    bmhd = BitMapHeader(width, height, 0, 0, depth,
                        0, 0, 0,
                        1, 1, width, height)
    cmap = [Color(*pal[i*3:(i+1)*3]) for i in range(colors)]
    body = c2p(pix, width, height, depth)

    ilbm = ILBM()
    ilbm.append(IffChunk('BMHD', bmhd))
    ilbm.append(IffChunk('CMAP', cmap))
    ilbm.append(IffChunk('BODY', body))
    ilbm.save(output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Converts an image to ILBM format.')
    parser.add_argument('input', metavar='INPUT', type=str,
                        help='Input image filename.')
    parser.add_argument('output', metavar='OUTPUT', type=str,
                        help='Input image filename.')
    args = parser.parse_args()

    convert(args.input, args.output)
