#!/usr/bin/env python3

from PIL import Image
from utils import constrain
import sys


if __name__ == '__main__':
    image = Image.open(sys.argv[1])
    image = image.convert('L')

    name = sys.argv[2]
    width, height = image.size

    print(f'#define {name}_width {width}')
    print(f'#define {name}_height {height}')
    print()
    print(f'static u_short {name}[{name}_width * {name}_height] = {{')

    for y in range(height):
        sys.stdout.write(' ')
        for x in range(width):
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
            sys.stdout.write(' 0x%04x,' % (u * 128 + v))
        sys.stdout.write('\n')

    print('};')
