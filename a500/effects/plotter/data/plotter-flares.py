#!/usr/bin/env python3 -B

from PIL import Image
from utils import lerp, sq, constrain
from math import sqrt

if __name__ == "__main__":
    D = 0.9
    SIZE = 16
    data = []
    pal = []

    for i in range(8):
        c = int(lerp(0, 255, float(i) / 7))
        pal.extend([c, c, c])

    im = Image.new('L', (SIZE, SIZE * 8))
    im.putpalette(pal)
    pix = im.load()

    for size in range(8, 16):
        r = lerp(2.0, 1.0, float(size - 8) / 7)

        for i in range(SIZE):
            for j in range(SIZE):
                u = lerp(-r, r, float(i) / (SIZE - 1))
                v = lerp(-r, r, float(j) / (SIZE - 1))

                d = sq(1.4 - constrain(sqrt(sq(u) + sq(v)), 0.0, 1.4))

                pix[i, j + (size - 8) * SIZE] = int(constrain(d, 0.0, 1.0) * 7)

    im.save("plotter-flares.png", "PNG")
