#!/usr/bin/env python3
#
# Copyright (c) 2024 Krystian Bacławski aka Cahir
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from PIL import Image
from os import path
import argparse
import sys


def closest(c, cols):
    i = -1
    dist = 256 * 256 * 3
    for col, idx in cols.items():
        r = c[0] - col[0]
        g = c[1] - col[1]
        b = c[2] - col[2]
        d = r * r + g * g + b * b
        if d < dist:
            dist = d
            i = idx
    return i


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description='Split pallette per image line into colors and raw data.')
    parser.add_argument('--force', action='store_true',
                        help='Ignore color issues.')
    parser.add_argument('ncols', metavar='N', type=int,
                        help='Number of colors to be changed per scanline.')
    parser.add_argument('filename', type=str,
                        help='Image where first N columns are taken by colors '
                             + 'and the rest by image data.')
    args = parser.parse_args()

    img = Image.open(args.filename)
    img = img.convert('RGB')
    width, height = img.size
    data_width = width - args.ncols

    if data_width & 15:
        sys.exit(f'Please fix the image: image data width ({data_width}) '
                 + 'is not divisible by 16!')

    img_pal = Image.new('RGB', (args.ncols, height))
    img_data = Image.new('P', (data_width, height))

    pix = img.load()
    pal = img_pal.load()
    data = img_data.load()

    issues = False
    for y in range(height):
        cols = dict()
        for x in range(width):
            c = pix[x, y]

            # palette columns
            if x < args.ncols:
                pal[x, y] = c
                if c not in cols:
                    cols[c] = x
                continue

            # image columns
            ci = cols.get(c, -1)
            if ci < 0:
                print(f'Please fix the image at (x:{x},y:{y}) where '
                      + f'{c} not in {list(cols.keys())}!')
                if args.force:
                    ci = closest(c, cols)
                    print(f'Replaced with {pix[ci, y]}!')
            if ci >= 0:
                data[x - args.ncols, y] = ci

    if issues and not args.force:
        sys.exit('Image needs to be fixed!')

    img_data_pal = []
    for i in range(0, 256, 256 // args.ncols):
        img_data_pal.extend([i, i, i])
    img_data.putpalette(img_data_pal)

    basename = path.splitext(args.filename)[0]
    img_pal.save(basename + '-pal.png')
    img_data.save(basename + '-data.png')
