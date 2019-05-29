#!/usr/bin/env python3 -B

from array import array
from uvmap import UVMap, Anamorphosis


def scramble_rgb12(uvmap):
  out = array("H")

  for x in uvmap:
    out.append(x * 2)

  return out


if __name__ == "__main__":
  uvmap = UVMap(80, 64)
  uvmap.generate(Anamorphosis, (0.25, 1.6 + 0.25, 0.25, 1.28 + 0.25))
  uvmap.save('uvmap-rgb', scramble_rgb12)
  uvmap.save_uv('uvmap-rgb')
