#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, FancyEye


def scramble_cmap4(uvmap):
    out = array("H")

    i = 0
    while i < len(uvmap):
        out.append(uvmap[i + 0])
        out.append(uvmap[i + 1])
        out.append(uvmap[i + 4])
        out.append(uvmap[i + 5])
        out.append(uvmap[i + 2])
        out.append(uvmap[i + 3])
        out.append(uvmap[i + 6])
        out.append(uvmap[i + 7])
        i += 8

    return out


if __name__ == "__main__":
    uvmap = UVMap(160, 100)
    uvmap.generate(FancyEye, (-1.6, 1.6, -1.0, 1.0))
    uvmap.save('uvmap', scramble_cmap4)
    uvmap.save_uv('uvmap')
