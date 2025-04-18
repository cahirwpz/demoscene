#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, HotMagma
from math import atan2, pi
from utils import dist
import sys


def Polar(x, y):
    a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0)
    return (a * 0.5 / pi, r / 2.56)


def scramble_rgb12(uvmap):
    out = array("H")

    for uv in uvmap:
        u = (uv >> 7) & 0x7f
        v = uv & 0x7f

        out.append(((u * 2) << 8) | (v * 2))

    return out


if __name__ == "__main__":
    uvmap = UVMap(80 * 2, 64 * 2)
    uvmap.generate(HotMagma, (-0.5, 0.5, -0.4, 0.4))
    # uvmap.generate(Polar, (-1.6, 1.6, -1.28, 1.28))
    uvmap.save(sys.argv[1], scramble_rgb12)
