#!/usr/bin/env python

from math import floor, atan2, cos, sin, sqrt
from array import array


def frpart(x):
  return x - floor(x)


def lerp(lo, hi, step):
  return lo + (hi - lo) * step


def dist(x1, y1, x2, y2):
  dx = x2 - x1
  dy = y2 - y1
  return sqrt(dx * dx + dy * dy)


def generate(width, height, fn):
  uvmap = array("B")

  for j in range(height):
    for i in range(width):
      x = lerp(-1.0, 1.0, float(i) / width)
      y = lerp(-1.0, 1.0, float(j) / height)

      (u, v) = fn(x, y)

      uvmap.append(int(frpart(u) * 255))
      uvmap.append(int(frpart(v) * 255))

  return uvmap


def scramble(uvmap):
  out = array("B")

  i = 0
  while i < len(uvmap):
    for j in range(4):
      out.append(uvmap[i + j])
    for j in range(4):
      out.append(uvmap[i + j + 8])
    for j in range(4):
      out.append(uvmap[i + j + 4])
    for j in range(4):
      out.append(uvmap[i + j + 12])
    i += 16

  return out


def FancyEye(x, y):
  a = atan2(x, y)
  r = dist(x, y, 0.0, 0.0)

  if r == 0:
    return (0, 0)

  u = 0.04 * y + 0.06 * cos(a * 3.0) / r
  v = 0.04 * x + 0.06 * sin(a * 3.0) / r

  return (u, v)


if __name__ == "__main__":
  with open("data/uvmap.bin", "w") as f:
    uvmap = generate(160, 128, FancyEye)
    scramble(uvmap).tofile(f)
