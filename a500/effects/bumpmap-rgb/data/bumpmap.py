#!/usr/bin/env python3

import sys
import os
from PIL import Image
from array import array
from utils import constrain


if __name__ == "__main__":
    image = Image.open(sys.argv[1])
    image = image.convert("L")

    bumpmap = array("H")
    name = os.path.splitext(sys.argv[1])[0] + ".bin"

    for y in range(image.size[1]):
        for x in range(image.size[0]):
            this = image.getpixel((x, y))
            if y < image.size[1] - 1:
                down = image.getpixel((x, y + 1))
            else:
                down = this
            if x < image.size[0] - 1:
                right = image.getpixel((x + 1, y))
            else:
                right = this
            # scale down the difference between pixels
            du = (this - down) * 0.25
            dv = (this - right) * 0.25
            # light texture size is 128 * 128
            u = (constrain(int(du), -64, 63) + y) & 127
            v = (constrain(int(dv), -64, 63) + x) & 127
            bumpmap.append(u * 128 + v)

    bumpmap.byteswap()

    with open(name, "w") as f:
        bumpmap.tofile(f)
