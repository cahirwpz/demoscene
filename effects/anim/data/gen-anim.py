#!/usr/bin/env python3

from PIL import Image
import os.path
import sys


if __name__ == '__main__':
    img = Image.open(sys.argv[1])
    img.load()

    name = os.path.splitext(os.path.basename(sys.argv[1]))[0]

    frames = 0

    for i in range(65536):
        try:
            img.seek(i)
        except EOFError:
            break
        frames += 1

    for n in range(frames):
        print('static u_char _%s_frame%d[] = {' % (name, n))
        img.seek(n)
        pix = img.load()
        for y in range(img.size[1]):
            line = []
            p = 1
            for x in range(img.size[0]):
                if p == pix[x, y]:
                    continue
                p = pix[x, y]
                line.append(x)
            line.insert(0, len(line))
            print('  %s,' % ', '.join(map(str, line)))
        print('};')
        print('')

    print('static AnimSpanT %s = {' % name)
    print('  .width = %d, .height = %d,' % img.size)
    print('  .current = 0, .count = %d,' % frames)
    print('  .frame = {')
    for n in range(frames):
        print('    _%s_frame%d,' % (name, n))
    print('  }')
    print('};')
