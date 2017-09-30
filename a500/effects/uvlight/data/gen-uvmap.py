#!/usr/bin/env python -B

from array import array
from uvmap import UVMap, Anamorphosis


def scramble_rgb12(uvmap):
    out = array('H')

    for x in uvmap:
        out.append(((x * 2) & 0xFEFE) + (x >> 15))

    return out


if __name__ == "__main__":
    uvmap = UVMap(80, 64)
    uvmap.load_uv('torus')
    uvmap.save('torus-map', scramble_rgb12, scale=128)
