#!/usr/bin/env python3

import argparse
import os.path
import sys
from PIL import Image
from utils import lerp, ccir601


def getcolors(im):
    pal = im.getpalette()
    return [(pal[i * 3], pal[i * 3 + 1], pal[i * 3 + 2])
            for _, i in im.getcolors()]


def gradient(pal1, pal2, path_out):
    im = Image.new('RGB', (16, 16))
    pix = im.load()

    for y in range(16):
        for x in range(16):
            r = lerp(pal1[x][0], pal2[x][0], float(y) / 15)
            g = lerp(pal1[x][1], pal2[x][1], float(y) / 15)
            b = lerp(pal1[x][2], pal2[x][2], float(y) / 15)

            pix[x, y] = (int(r), int(g), int(b))

    im.save(path_out, 'PNG')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate gradient between given pallete.')
    parser.add_argument('--palette', type=str, required=True)
    parser.add_argument('input', metavar='INPUT', type=str)
    parser.add_argument('output', metavar='OUTPUT', type=str)
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        raise SystemExit('Input file does not exists!')

    if not os.path.isfile(args.palette):
        raise SystemExit('Palette file does not exists!')

    im1 = Image.open(args.input)
    im2 = Image.open(args.palette)

    pal_src = sorted(getcolors(im1), key=ccir601)
    pal_dst = sorted(getcolors(im2), key=ccir601)

    gradient(pal_src, pal_dst, args.output)
