#!/usr/bin/env python -B

from math import atan2, cos, sin, pi
from utils import dist, lerp
from array import array
import Image
import subprocess


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


def scramble_rgb12(uvmap):
    out = array("H")

    for x in uvmap:
        out.append(x * 2)

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


def HotMagma(x, y):
    a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0)

    if r == 0:
        return (0, 0)

    u = 0.5 * a / pi
    v = sin(2.0 * r)

    return (u, v)


class UVMap(object):
    def __init__(self, width, height):
        self.uvmap = None
        self.width = width
        self.height = height

    def generate(self, fn, view):
        self.uvmap = []

        for j in range(self.height):
            for i in range(self.width):
                x = lerp(view[0], view[1], float(i) / self.width)
                y = lerp(view[2], view[3], float(j) / self.height)

                (u, v) = fn(x, y)

                self.uvmap.append((int(u * 256) & 255, int(v * 256) & 255))

    def save(self, name, fn):
        im = Image.new('I', (self.width, self.height))
        data = array("H")
        for u, v in self.uvmap:
            data.append((u & 127) * 128 + (v & 127))
        im.putdata(fn(data))
        im.save('%s.png' % name, 'PNG')
        subprocess.call(['optipng', '-o7', '%s.png' % name])

    def save_uv(self, name):
        im = Image.new('L', (self.width, self.height))
        im.putdata([u for u, _ in self.uvmap])
        im.save('%s-u.png' % name, 'PNG')
        im = Image.new('L', (self.width, self.height))
        im.putdata([v for _, v in self.uvmap])
        im.save('%s-v.png' % name, 'PNG')
        # subprocess.call(['optipng', '-o7', '%s-u.png' % name])
        # subprocess.call(['optipng', '-o7', '%s-v.png' % name])


if __name__ == "__main__":
    uvmap = UVMap(160, 100)
    uvmap.generate(FancyEye, (-1.6, 1.6, -1.0, 1.0))
    uvmap.save('uvmap', scramble_cmap4)
    uvmap.save_uv('uvmap')
    raise SystemExit

    uvmap = UVMap(80, 64)
    uvmap.generate(Anamorphosis, (0.25, 1.6 + 0.25, 0.25, 1.28 + 0.25))
    uvmap.save('uvmap-rgb', scramble_rgb12)
    uvmap.save_uv('uvmap-rgb')
