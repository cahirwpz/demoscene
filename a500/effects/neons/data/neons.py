#!/usr/bin/env python2

from PIL import Image
from utils import lerp


if __name__ == "__main__":
    width = 320
    height = 256
    A = 128
    B = 64
    pal = []

    for i in range(2):
        pal.extend([0, 0, 0])

    for i in range(6):
        c = int(lerp(0, 15, float(i) / 5))
        c = (c << 4) | c
        pal.extend([0, c, c])

    for i in range(6):
        c = int(lerp(15, 0, float(i) / 5))
        c = (c << 4) | c
        pal.extend([0, c, c])

    for i in range(2):
        pal.extend([0, 0, 0])

    im = Image.new('L', (width, height))
    im.putpalette(pal)
    pix = im.load()

    for i in range(-B, B):
        for j in range(height):
            pix[i + width / 2, j] = j % 15 + 1

    W = width / 2 - B
    minY = 0
    maxY = height

    for i in range(W):
        y0 = float(i * A) / W - A
        y1 = (height - 1) - y0
        dy = y1 - y0

        v0 = (minY - y0) * 256 / dy
        v1 = maxY + (maxY - y1) * 256 / dy

        for j in range(height):
            v = int(lerp(v0, v1, float(j) / 255))
            pix[i, j] = v % 15 + 1
            pix[width - i - 1, j] = v % 15 + 1

    im.save("neons.png", "PNG")
