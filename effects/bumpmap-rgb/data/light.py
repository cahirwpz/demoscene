#!/usr/bin/env python3

from utils import constrain, lerp, dist
import sys


if __name__ == '__main__':
    size = (128, 128)

    print('const u_short light_w = 128;')
    print('const u_short light_h = 128;')
    print('')
    print('u_char %s[] = {' % sys.argv[1])

    for i in range(size[0]):
        sys.stdout.write(' ')
        for j in range(size[1]):
            x = lerp(-1.5, 1.5, float(i) / size[0])
            y = lerp(-1.5, 1.5, float(j) / size[1])
            pixel = 1.0 - dist(0.0, 0.0, x, y)
            sys.stdout.write(' 0x%02x,' % int(
                constrain(pixel, 0.0, 1.0) * 255))
        sys.stdout.write('\n')
    print('};')
