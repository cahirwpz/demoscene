#!/usr/bin/env python2

from PIL import Image
from utils import constrain, lerp, dist


if __name__ == "__main__":
    size = (128, 128)
    data = []

    for i in range(size[0]):
        for j in range(size[1]):
            x = lerp(-1.5, 1.5, float(i) / size[0])
            y = lerp(-1.5, 1.5, float(j) / size[1])

            pixel = 1.0 - dist(0.0, 0.0, x, y)

            data.append(int(constrain(pixel, 0.0, 1.0) * 255))

    light = Image.new('L', size)
    light.putdata(data)
    light.save("light.png", "PNG")
