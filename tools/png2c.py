#!/usr/bin/env python3

from PIL import Image
from array import array
from math import ceil, log
import argparse


def dim(s):
    w, h, d = s.split('x')
    return int(w), int(h), int(d)


def parse(spec, *desc, **kwargs):
    param = dict(kwargs)

    sl = []
    for s in spec.split(','):
        if s[0] in '+-':
            flag = s[1:]
            if flag not in kwargs:
                raise SystemExit('Unknown flag {}!'.format(flag))
            param[flag] = bool(s[0] == '+')
        else:
            sl.append(s)

    if len(sl) != len(desc):
        raise SystemExit('Got {}, but expected {}!'.format(s, desc))

    for value, (name, cast) in zip(sl, desc):
        try:
            if type(name) == tuple:
                for n, v in zip(name, cast(value)):
                    param[n] = v
            else:
                param[name] = cast(value)
        except ValueError:
            raise SystemExit('Got {}, but expected {}!'.format(v, cast))

    return param


def c2p(pix, width, height, depth):
    data = array('H')
    padding = array('B', [0 for _ in range(16 - (width & 15))])

    for offset in range(0, width * height, width):
        row = pix[offset:offset + width]
        if width & 15:
            row.extend(padding)
        for p in range(depth):
            bits = [(byte >> p) & 1 for byte in row]
            for i in range(0, width, 16):
                word = 0
                for j in range(16):
                    word = word * 2 + bits[i + j]
                data.append(word)

    return data


def do_bitmap(im, desc):
    param = parse(desc, ('name', str), (('width', 'height', 'depth'), dim),
                  interleaved=False)

    name = param['name']
    has_width = param['width']
    has_height = param['height']
    has_depth = param['depth']
    interleaved = param['interleaved']

    pix = array('B', im.getdata())

    width, height = im.size
    colors = im.getextrema()[1] + 1
    depth = int(ceil(log(colors, 2)))

    if width != has_width or height != has_height or depth != has_depth:
        raise SystemExit(
            'Image is {}, expected {}!'.format(
                'x'.join(map(str, [width, height, depth])),
                'x'.join(map(str, [has_width, has_height, has_depth]))))

    bytesPerRow = ((width + 15) & ~15) // 8
    wordsPerRow = bytesPerRow // 2
    bplSize = bytesPerRow * height
    bpl = c2p(pix, width, height, depth)

    print('static __data_chip u_short _%s_bpl[] = {' % name)
    if interleaved:
        for i in range(0, depth * wordsPerRow * height, wordsPerRow):
            words = ['0x%04x' % bpl[i + x] for x in range(wordsPerRow)]
            print('  %s,' % ','.join(words))
    else:
        for i in range(0, depth * wordsPerRow, wordsPerRow):
            for y in range(height):
                words = ['0x%04x' % bpl[i + x] for x in range(wordsPerRow)]
                print('  %s,' % ','.join(words))
                i += wordsPerRow * depth
    print('};')
    print('')

    print('BitmapT %s = {' % name)
    print('  .width = %d,' % width)
    print('  .height = %d,' % height)
    print('  .depth = %s,' % depth)
    print('  .bytesPerRow = %d,' % bytesPerRow)
    print('  .bplSize = %d,' % bplSize)
    flags = ['BM_DISPLAYABLE']
    if interleaved:
        flags.append('BM_INTERLEAVED')
    print('  .flags = %s,' % '|'.join(flags))
    print('  .compression = 0,')
    print('  .palette = NULL,')
    print('  .pchgTotal = 0,')
    print('  .pchg = NULL,')
    print('  .planes = {')
    for i in range(depth):
        if interleaved:
            offset = i * bytesPerRow
        else:
            offset = i * bplSize
        print('    (void *)_%s_bpl + %d,' % (name, offset))
    print('  }')
    print('};')


def do_palette(im, desc):
    param = parse(desc, ('name', str), ('colors', int))

    name = param['name']
    has_colors = param['colors']

    pal = im.getpalette()
    colors = im.getextrema()[1] + 1

    if pal is None:
        raise SystemExit('Image has no palette!')
    if colors != has_colors:
        raise SystemExit(
            'Image has {} colors, expected {}!'.format(colors, has_colors))

    cmap = [pal[i * 3:(i + 1) * 3] for i in range(colors)]

    print('PaletteT %s = {' % name)
    print('  .count = %d,' % len(cmap))
    print('  .colors = {')
    for r, g, b in cmap:
        print('    {%d, %d, %d},' % (r, g, b))
    print('  }')
    print('};')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Converts an image to bitmap.')
    parser.add_argument('-b', '--bitmap', type=str,
                        help='Output Amiga bitmap '
                             '(desc:name,dimensions).')
    parser.add_argument('-p', '--palette', type=str,
                        help='Output Amiga palette '
                             '(desc:name,colors).')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='Input image filename.')
    args = parser.parse_args()

    im = Image.open(args.path)

    if im.mode not in ['1', 'L', 'P']:
        raise SystemExit('Only 8-bit images supported.')

    if args.palette:
        do_palette(im, args.palette)
        print('')

    if args.bitmap:
        do_bitmap(im, args.bitmap)
        print('')
