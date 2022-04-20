#!/usr/bin/env python3

import csv
import sys
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
    frames = read_anim(sys.argv[1])

    print('#define frame_count', len(frames))

    print('static u_char verts_in_frame[] = {')
    for polys in frames:
        n = sum(len(poly) for poly in polys) + 1
        print('%d,' % n)
    print('};')

    width = 320
    height = 180
    wrapper = TextWrapper(initial_indent='  ',
                          subsequent_indent='    ',
                          width=80)

    print('static u_short verts[] = {')
    for fn, polys in enumerate(frames):
        print('  /* frame %d */' % fn)
        for poly in polys:
            verts = []
            for x, y in poly:
                x = min(max(0, x), width - 1)
                y = min(max(0, y), height - 1)
                verts.append(str(y * width + x))
            verts.append('65535')
            for line in wrapper.wrap(', '.join(verts) + ','):
                print(line)
    print('};')
