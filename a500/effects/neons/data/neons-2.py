#!/usr/bin/env python2

from PIL import Image
from subprocess import call
from utils import lerp, dist, saw
from math import atan2, sin, cos, pi


def Neon1(i, j):
    x = lerp(-1.5, 1.5, float(i) / width)
    y = lerp(-2.0, -0.5, float(j) / height)

    # a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0)

    if r == 0:
        return 0

    return int(512 / r)


def Neon2(i, j):
    x = lerp(-1.0, 1.0, float(i) / width)
    y = lerp(-1.0, 1.0, float(j) / height)

    a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0) + 0.2 * sin(7 * a)

    if r == 0:
        return 0

    return int(16 / r)


def Neon3(i, j):
    x = lerp(-1.0, 1.0, float(i) / width)
    y = lerp(-0.8, 1.2, float(j) / height)

    a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0) + 0.2 * saw(5.0 * a /
                                         (2.0 * pi)) + 0.1 * abs(sin(5.0 * a))
    #r = dist(x, y, 0.0, 0.0) + 0.2 * abs(sin(5.0 * a))

    if r == 0:
        return 0

    return int(r * 64)


if __name__ == "__main__":
    width = 320
    height = 256
    A = 128
    B = 64
    pal = []

    for i in range(2):
        pal.extend([0, 0, 0])

    for i in range(6):
        c = int(lerp(0, 15, float(i) / 5))
        c = (c << 4) | c
        pal.extend([0, c, c])

    for i in range(6):
        c = int(lerp(15, 0, float(i) / 5))
        c = (c << 4) | c
        pal.extend([0, c, c])

    for i in range(2):
        pal.extend([0, 0, 0])

    im = Image.new('L', (width, height))
    im.putpalette(pal)
    pix = im.load()

    for j in range(height):
        for i in range(width):
            pix[i, j] = Neon3(i, j) % 16

    im.save("neons-2.png", "PNG")
