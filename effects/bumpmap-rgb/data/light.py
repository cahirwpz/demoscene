#!/usr/bin/env python3

from utils import constrain, lerp, dist
import sys


if __name__ == '__main__':
    name = sys.argv[1]
    width = 128
    height = 128

    print(f'#define {name}_width {width}')
    print(f'#define {name}_height {height}')
    print('')
    print(f'static u_char {name}[{name}_width * {name}_height] = {{')

    for j in range(height):
        sys.stdout.write(' ')
        for i in range(width):
            x = lerp(-1.5, 1.5, float(i) / width)
            y = lerp(-1.5, 1.5, float(j) / height)
            pixel = 1.0 - dist(0.0, 0.0, x, y)
            sys.stdout.write(' 0x%02x,' % int(
                constrain(pixel, 0.0, 1.0) * 255))
        sys.stdout.write('\n')
    print('};')
