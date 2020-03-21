#!/usr/bin/env python3

from PIL import Image
import argparse


def parse(s, desc):
    sl = s.split(',')
    if len(sl) != len(desc):
        raise SystemExit('Got {}, but expected {}!'.format(s, desc))

    values = []
    for v, cast in zip(sl, desc):
        try:
            values.append(cast(v))
        except ValueError:
            raise SystemExit('Got {}, but expected {}!'.format(v, cast))

    return values


def do_palette(im, desc):
    name, color_count = parse(desc, [str, int])

    pal = im.getpalette()
    colors = im.getextrema()[1] + 1

    if pal is None:
        raise SystemExit('Image has no palette!')
    if colors != color_count:
        raise SystemExit(
            'Image has {} colors, expected {}!'.format(colors, color_count))

    cmap = [pal[i * 3:(i + 1) * 3] for i in range(colors)]

    print('PaletteT %s = {' % name)
    print('  .count = %d,' % len(cmap))
    print('  .colors = {')
    for r, g, b in cmap:
        print('    {%d, %d, %d},' % (r, g, b))
    print('  }')
    print('};')


def convert(path, palette):
    im = Image.open(path)

    if im.mode not in ['1', 'L', 'P']:
        raise SystemExit('Only 8-bit images supported.')

    if palette:
        do_palette(im, palette)
        print('')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Converts an image to bitmap.')
    parser.add_argument('-p', '--palette', type=str,
                        help='Output Amiga palette (desc:name,colors).')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='Input image filename.')
    args = parser.parse_args()

    convert(args.path, args.palette)
