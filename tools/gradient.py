#!/usr/bin/env python3

import argparse
import os.path
import sys

from math import floor
from PIL import Image
from utils import constrain, frpart, lerp, ccir601


def rgb_to_hsv(c):
    r, g, b = c

    r /= 255.0
    g /= 255.0
    b /= 255.0

    c_max = max([r, g, b])
    c_min = min([r, g, b])
    diff = c_max - c_min

    h, s, v = 0.0, 0.0, c_max

    if c_max > 0.0:
        s = diff / c_max

    if s != 0.0:
        if r == c_max:
            h = (g - b) / diff
        elif g == c_max:
            h = 2.0 + (b - r) / diff
        else:
            h = 4.0 + (r - g) / diff
        h /= 6.0
        if h < 0.0:
            h += 1.0

    return (h, s, v)


def hsv_to_rgb(c):
    h, s, v = c

    if s == 0.0:
        v = int(v * 255.0)
        return (v, v, v)

    if h == 1.0:
        h = 0.0

    h *= 6.0

    hi = floor(h)
    hf = h - hi

    p = v * (1.0 - s)
    q = v * (1.0 - s * hf)
    t = v * (1.0 - s * (1.0 - hf))

    if hi == 0:
        r, g, b = v, t, p
    elif hi == 1:
        r, g, b = q, v, p
    elif hi == 2:
        r, g, b = p, v, t
    elif hi == 3:
        r, g, b = p, q, v
    elif hi == 4:
        r, g, b = t, p, v
    else:
        r, g, b = v, p, q

    return (int(r * 255.0), int(g * 255.0), int(b * 255.0))


def getcolors(path):
    im = Image.open(path)
    pal = im.getpalette()
    colors = [(pal[i * 3], pal[i * 3 + 1], pal[i * 3 + 2])
              for _, i in im.getcolors()]
    return sorted(colors, key=ccir601)


def invert(pal):
    return [(255 - r, 255 - g, 255 - b) for r, g, b in pal]


def modify_hsv(pal, dh, ds, dv):
    out = []
    for h, s, v in map(rgb_to_hsv, pal):
        h = frpart(h + dh)
        s = constrain(s + ds, 0.0, 1.0)
        v = constrain(v + dv, 0.0, 1.0)
        out.append(hsv_to_rgb((h, s, v)))
    return out


def gradient(pal1, pal2, path_out):
    im = Image.new('RGB', (16, 16))
    pix = im.load()

    for y in range(16):
        for x in range(16):
            r = lerp(pal1[x][0], pal2[x][0], float(y) / 15)
            g = lerp(pal1[x][1], pal2[x][1], float(y) / 15)
            b = lerp(pal1[x][2], pal2[x][2], float(y) / 15)

            pix[x, y] = (int(r), int(g), int(b))

    im.save(path_out, 'PNG')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate color gradient from given source pallete. '
                    'Destination palette may be delivered from source one, '
                    'otherwise is taken from source and modified.')
    parser.add_argument('--palette', type=str,
                        help='Take destination palette from file.')
    parser.add_argument('--invert', action='store_true',
                        help='Invert colors (aka negative).')
    parser.add_argument('--hue', type=float, default=0.0,
                        help='Change Hue component in HSV space. The value '
                             'wraps around in [0.0, 1.0] range.')
    parser.add_argument('--saturation', type=float, default=0.0,
                        help='Change Saturation component in HSV space. The '
                             'value is constrained within [0.0, 1.0] range.')
    parser.add_argument('--value', type=float, default=0.0,
                        help='Change Value component in HSV space. The '
                             'value is constrained within [0.0, 1.0] range.')
    parser.add_argument('input', metavar='INPUT', type=str,
                        help='Image from which source palette will be '
                             'extracted.')
    parser.add_argument('output', metavar='OUTPUT', type=str,
                        help='Output gradient file. Image of N x 16 size '
                             'in RGB space. Each line represents single '
                             'palette. There are 14 transitions between '
                             'source and destination palette.')
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        raise SystemExit('Input file does not exists!')

    if args.palette and not os.path.isfile(args.palette):
        raise SystemExit('Palette file does not exists!')

    pal_src = getcolors(args.input)
    if args.palette:
        pal_dst = getcolors(args.palette)
    if args.invert:
        pal_dst = invert(pal_src)
    if args.hue or args.saturation or args.value:
        pal_dst = modify_hsv(pal_src, args.hue, args.saturation, args.value)

    gradient(pal_src, pal_dst, args.output)
