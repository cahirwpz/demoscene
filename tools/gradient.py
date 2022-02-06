#!/usr/bin/env python3 -B

from PIL import Image
from utils import lerp, ccir601
import sys


def getcolors(im):
    pal = im.getpalette()
    return [(pal[i * 3], pal[i * 3 + 1], pal[i * 3 + 2])
            for _, i in im.getcolors()]


if __name__ == "__main__":
    im1 = Image.open(sys.argv[1])
    im2 = Image.open(sys.argv[2])

    pal1 = sorted(getcolors(im1), key=ccir601)
    pal2 = sorted(getcolors(im2), key=ccir601)

    im = Image.new('RGB', (16, 16))
    pix = im.load()

    for y in range(16):
        for x in range(16):
            r = lerp(pal1[x][0], pal2[x][0], float(y) / 15)
            g = lerp(pal1[x][1], pal2[x][1], float(y) / 15)
            b = lerp(pal1[x][2], pal2[x][2], float(y) / 15)

            pix[x, y] = (int(r), int(g), int(b))

    im.save(sys.argv[3], 'PNG')
