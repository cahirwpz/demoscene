#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap
import sys


def scramble_rgb12(uvmap):
    out = array('H')

    for x in uvmap:
        out.append(((x * 2) & 0xFEFE) + (x >> 15))

    return out


if __name__ == "__main__":
    uvmap = UVMap(80, 64)
    uvmap.load_uv(sys.argv[1])
    uvmap.save(sys.argv[1] + '-map', scramble_rgb12, scale=128)
