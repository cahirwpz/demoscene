#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, Anamorphosis
import sys


def scramble_rgb12(uvmap):
    out = array("H")

    for x in uvmap:
        out.append(x * 2)

    return out


if __name__ == "__main__":
    uvmap = UVMap(80 * 2, 64 * 2)
    uvmap.generate(Anamorphosis, (-3.2, 3.2, -2.56, 2.56))
    uvmap.save(sys.argv[1], scramble_rgb12)
