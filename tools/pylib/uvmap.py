from __future__ import print_function

from math import atan2, cos, sin, pi, sqrt, tan
from utils import dist, lerp, frpart
from array import array
from PIL import Image


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


def Ball(x, y):
    r = dist(x, y, 0.0, 0.0)
    r2 = r * r

    try:
        v = x * (1.33 - sqrt(1.0 - r2)) / (r2 + 1.0)
        u = y * (1.33 - sqrt(1.0 - r2)) / (r2 + 1.0)
        return (u, v)
    except ValueError:
        pass


def Butterfly(x, y):
    x = abs(x)
    y = abs(y)

    p = 0.1
    q = 0.2

    a = atan2(x, y)
    r = dist(x, y, 0.0, 0.0)

    if r == 0.0 or y < p * x or x < q * y:
        return None

    u = .5 * cos(3 * a) + r * tan(a) * 0.1
    v = .5 * sin(3 * a) + r / (1 if tan(a) == 0 else tan(a))

    return (u, v)


class UVMap(object):

    def __init__(self, width, height, texsize=128):
        self.umap = array('f', [0.0 for i in range(width * height)])
        self.vmap = array('f', [0.0 for i in range(width * height)])
        self.mask = array('B', [0 for i in range(width * height)])
        self.width = width
        self.height = height
        self.texsize = texsize

    def put(self, x, y, value):
        i = x + y * self.width
        if value:
            self.umap[i] = value[0]
            self.vmap[i] = value[1]
            self.mask[i] = 1
        else:
            self.mask[i] = 0

    def get(self, x, y):
        i = x + y * self.width
        return (self.umap[i], self.vmap[i])

    def generate(self, fn, view):
        for j in range(self.height):
            for i in range(self.width):
                x = lerp(view[0], view[1], float(i) / self.width)
                y = lerp(view[2], view[3], float(j) / self.height)
                self.put(i, j, fn(x, y))

    def _load_map(self, name):
        im = Image.open(name)
        if im.size[0] != self.width or im.size[1] != self.height:
            raise RuntimeError('Image size does not match uvmap size!')
        return im.convert('RGB')

    def load_uv(self, name):
        im_u = self._load_map(name + '-u.png')
        im_v = self._load_map(name + '-v.png')
        for i, (uc, vc) in enumerate(zip(im_u.getdata(), im_v.getdata())):
            u, v = uc[0], vc[0]
            if uc == (u, u, u) and vc == (v, v, v):
                self.mask[i] = 1
                self.umap[i] = float(u) / 256.0
                self.vmap[i] = float(v) / 256.0

    def save(self, name, fn=None, scale=256):
        size = self.texsize
        data = array('H')
        for i in range(self.width * self.height):
            if self.mask[i]:
                u = int(frpart(self.umap[i]) * scale) % size
                v = int(frpart(self.vmap[i]) * scale) % size
                data.append(u * size + v)
            else:
                data.append(0xffff)
        if fn:
            data = fn(data)
        print('u_short %s[%d] = {' % (name, self.width * self.height))
        for i in range(0, self.width * self.height, self.width):
            row = ['0x%04x' % val for val in data[i:i + self.width]]
            print('  %s,' % ', '.join(row))
        print('};')

    def save_uv(self, name):
        im = Image.new('L', (self.width, self.height))
        im.putdata([frpart(u) * 256 for u in self.umap])
        im.save(name + '-u.png', 'PNG')
        im = Image.new('L', (self.width, self.height))
        im.putdata([frpart(v) * 256 for v in self.vmap])
        im.save(name + '-v.png', 'PNG')
