#!/usr/bin/env python3 -B

from uvmap import UVMap
from array import array
from utils import dist, lerp, frpart
from math import atan2, cos, sin, pi, sqrt

import sys


# Dummy UV-Map for testing
def Copy(x, y):
    return (y, x)


class Lens(object):

    # linear - zoom factor
    # square - distortion factor
    def __init__(self, linear, square):
        self.linear = linear
        self.square = square

    def __call__(self, x, y):
        r = dist(x, y, 0, 0)
        if r > 1:
            return None
        r2 = r*r
        r = r*self.linear + r2*self.square

        try:
            phi = atan2(x, y)
            u = r * cos(phi)
            v = r * sin(phi)
            return (u+0.5, v+0.5)
        except ValueError:
            pass


class NonUniformUVMap(UVMap):

    def __init__(self, width, height, utexsize=128, vtexsize=128):
        self.umap = array('f', [0.0 for i in range(width * height)])
        self.vmap = array('f', [0.0 for i in range(width * height)])
        self.mask = array('B', [0 for i in range(width * height)])
        self.width = width
        self.height = height
        self.utexsize = utexsize
        self.vtexsize = vtexsize

    def save(self, name, fn=None, uscale=256, vscale=256):
        data = array('H')
        for i in range(self.width * self.height):
            if self.mask[i]:
                u = int(frpart(self.umap[i]) * uscale) % self.utexsize
                v = int(frpart(self.vmap[i]) * vscale) % self.vtexsize
                data.append(u * self.vtexsize + v)
            else:
                data.append(0xffff)
        if fn:
            data = fn(data)
        w, h = self.width, self.height
        print(f'#define {name}_width {w}')
        print(f'#define {name}_height {h}')
        print()
        print(f'static u_short {name}[{name}_width * {name}_height] = {{')
        for i in range(0, w * h, self.width):
            row = ['0x%04x' % val for val in data[i:i + self.width]]
            print('  %s,' % ', '.join(row))
        print('};')


if __name__ == "__main__":
    uvmap = NonUniformUVMap(64, 64, utexsize=64, vtexsize=32)
    lens = Lens(0.2, 0.3)
    uvmap.generate(lens, (-1, 1, -1, 1))
    # uvmap.generate(Copy, (0, 1, 0, 1))
    uvmap.save(sys.argv[1], uscale=64, vscale=32)
