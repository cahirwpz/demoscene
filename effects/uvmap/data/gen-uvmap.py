#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, FancyEye
import sys


def scramble_cmap4(uvmap):
    # ([a b] [e f]) ([c d] [g h]) ([i j] [m n]) ([k l] [o p]) ...
    out = array('H')
    n = len(uvmap)

    for i in range(0, n, 8):
        out.append(uvmap[i + 0] * 2)
        out.append(uvmap[i + 1] * 2)
        out.append(uvmap[i + 4] * 2)
        out.append(uvmap[i + 5] * 2)
        out.append(uvmap[i + 2] * 2)
        out.append(uvmap[i + 3] * 2)
        out.append(uvmap[i + 6] * 2)
        out.append(uvmap[i + 7] * 2)

    return out


if __name__ == '__main__':
    uvmap = UVMap(160, 100)
    uvmap.generate(FancyEye, (-1.6, 1.6, -1.0, 1.0))
    uvmap.save(sys.argv[1], scramble_cmap4)
