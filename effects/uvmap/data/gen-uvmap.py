#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, Butterfly
import sys


def doit(uv):
    if uv == 0xffff:
        return uv
    return uv * 2


def scramble_cmap4(uvmap):
    # ([a b] [e f]) ([c d] [g h]) ([i j] [m n]) ([k l] [o p]) ...
    out = array('H')
    n = len(uvmap)

    for i in range(0, n, 8):
        a, b, c, d, e, f, g, h = map(doit, uvmap[i:i+8])
        out.extend([a, b, e, f, c, d, g, h])

    return out


if __name__ == '__main__':
    uvmap = UVMap(160, 100)
    uvmap.generate(Butterfly, (-1.6, 1.6, -1.0, 1.0))
    uvmap.save(sys.argv[1], scramble_cmap4)
