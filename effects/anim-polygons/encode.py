#!/usr/bin/env python3

import csv
import sys
import os.path
from math import log2, ceil
from itertools import chain
from textwrap import TextWrapper
import logging


def read_anim(path):
    frames = []

    with open(path) as csvfile:
        reader = csv.DictReader(csvfile)

        frame = 0
        shape = 0
        polys = []
        verts = []

        for row in reader:
            x, y, fn, sn = (int(row['x']), int(row['y']), int(row['fn']),
                            int(row['sn']))

            if fn != frame:
                if len(verts) < 3:
                    logging.warning(
                            f'(fn:{frame},sn:{shape}) is not a polygon!')
                else:
                    polys.append(verts)
                shape = sn
                verts = []

                frames.append(polys)
                frame = fn
                polys = []

            if sn != shape:
                if len(verts) < 3:
                    logging.warning(
                            f'(fn:{frame},sn:{shape}) is not a polygon!')
                else:
                    polys.append(verts)
                shape = sn
                verts = []

            verts.append((x, y))

        polys.append(verts)
        frames.append(polys)

    return frames


if __name__ == '__main__':
    path = sys.argv[1]
    frames = read_anim(path)
    name, _ = os.path.splitext(os.path.basename(path))

    Wrapper = TextWrapper(initial_indent=' ' * 2,
                          subsequent_indent=' ' * 4,
                          width=80)

    width = 320
    height = 180

    for fn, polys in enumerate(frames):
        count = sum(len(poly) for poly in polys) + 1

        print('static short %s_frame%d[] = {' % (name, fn))

        for poly in polys:
            verts = [len(poly)]
            for x, y in poly:
                x = min(max(0, x), width - 1)
                y = min(max(0, y), height - 1)
                verts.append(y * width + x)
            for line in Wrapper.wrap(', '.join(map(str, verts)) + ','):
                print(line)

        print('  0,')
        print('};')
        print('')

    print('#define %s_frames %d' % (name, len(frames)))
    print('')
    print('static short *%s_frame[] = {' % name)
    for fn, _ in enumerate(frames):
        print('  %s_frame%d,' % (name, fn))
    print('};')
