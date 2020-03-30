#!/usr/bin/env python3

from PIL import Image
from math import atan2, pi
import sys


WIDTH, HEIGHT = 38, 32

if __name__ == "__main__":
    im = Image.new('L', (WIDTH, HEIGHT))
    pix = im.load()
    for y in range(0, HEIGHT):
        for x in range(0, WIDTH):
            xi = x - WIDTH / 2
            yi = y - HEIGHT / 2
            v = int(abs((atan2(yi, xi) / pi) % 1.0) * 255.0)
            pix[x, y] = v
    im.save(sys.argv[1], 'PNG')
