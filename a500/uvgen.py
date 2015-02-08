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
  uvmap = array("H")

  for j in range(height):
    for i in range(width):
      x = lerp(-1.0, 1.0, float(i) / width)
      y = lerp(-1.0, 1.0, float(j) / height)

      (u, v) = fn(x, y)

      u = int(u * 256) & 127
      v = int(v * 256) & 127

      uvmap.append(u * 128 + v)

  return uvmap


def scramble(uvmap):
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

  out.byteswap()

  return out


def deinterleave(uvmap, width, height):
  out = array("H")

  for y in range(height):
    for x in range(0, width, 2):
      out.append(uvmap[width * y + x] * 2)
    for x in range(1, width, 2):
      out.append(uvmap[width * y + x] * 2)

  out.byteswap()

  return out


def FancyEye(x, y):
  a = atan2(x, y)
  r = dist(x, y, 0.0, 0.0)

  if r == 0:
    return (0, 0)

  u = 0.04 * y + 0.06 * cos(a * 3.0) / r
  v = 0.04 * x + 0.06 * sin(a * 3.0) / r

  return (u, v)


def Anamorphosis(x, y):
  a = atan2(x, y)
  r = dist(x, y, 0.0, 0.0)

  if r == 0:
    return (0, 0)

  u = cos(a) / (3.0 * r)
  v = sin(a) / (3.0 * r)

  return (u, v)

if __name__ == "__main__":
  with open("data/uvmap.bin", "w") as f:
    uvmap = generate(160, 100, FancyEye)
    scramble(uvmap).tofile(f)

  with open("data/uvmap-rgb.bin", "w") as f:
    uvmap = generate(80, 128, FancyEye)
    deinterleave(uvmap, 80, 128).tofile(f)
