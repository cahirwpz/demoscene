#!/usr/bin/env python3

from PIL import Image
from color import Color
from math import floor
import sys


COLORS = [Color(0.2, 0.7, 1.0), Color(0.2, 1.0, 1.0),
          Color(0.2, 0.7, 1.0), Color(0.6, 0.7, 0.6),
          Color(1.0, 0.7, 0.2), Color(1.0, 1.0, 0.5),
          Color(1.0, 0.7, 0.2), Color(0.6, 0.7, 0.6)]


def gradient(colors, v):
    v = v * len(colors)
    f = v - floor(v)
    i1 = int(floor(v))
    i2 = (i1 + 1) % len(colors)
    return Color.lerp(colors[i1], colors[i2], f)


if __name__ == "__main__":
    im = Image.new('RGB', (256, 1))
    pix = im.load()
    for x in range(256):
        pix[x, 0] = gradient(COLORS, x / 256.0).rgb24()
    im.save(sys.argv[1], 'PNG')
