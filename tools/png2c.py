#!/usr/bin/env python3

from PIL import Image
from array import array
from math import ceil, log
import argparse


def coerce(name, typ, value):
    try:
        return typ(value)
    except ValueError:
        raise SystemExit(f'{name}: could not convert {value} to {typ}!')


def convert(name, cast, value, result):
    if type(cast) == tuple:
        names = name.split(',')
        values = value.split('x')
        if len(names) > 1:
            assert len(names) == len(values)
            for n, c, v in zip(names, cast, values):
                result[n] = coerce(n, c, v)
        else:
            result[name] = tuple(coerce(n, c, v)
                                 for n, c, v in zip(name, cast, values))
    else:
        result[name] = coerce(name, cast, value)


def parse(desc, *params):
    mandatory = []
    optional = {}
    result = {}

    for param in params:
        if len(param) == 2:
            mandatory.append(param)
        elif len(param) == 3:
            name, cast, defval = param
            optional[name] = cast
            result[name] = defval
        else:
            raise RuntimeError

    desc = desc.split(',')

    for name, cast in mandatory:
        try:
            value = desc.pop(0)
        except ValueError:
            raise SystemExit(f'Missing {name} argument!')

        convert(name, cast, value, result)

    for s in desc:
        if not s:
            continue

        if s[0] in '+-':
            name, value = s[1:], bool(s[0] == '+')
        else:
            try:
                name, value = s.split('=', 1)
            except ValueError:
                raise SystemExit(f'Malformed optional argument {s}!')

        if name not in optional:
            raise SystemExit(f'Unknown optional argument {name}!')

        convert(name, optional[name], value, result)

    return result


def planar(pix, width, height, depth):
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


def chunky4(im):
    pix = im.load()
    width, height = im.size
    data = array('B')

    for y in range(height):
        for x in range(0, (width + 1) & ~1, 2):
            x0 = pix[x, y] & 15
            if x + 1 < width:
                x1 = pix[x + 1, y] & 15
            else:
                x1 = 0
            data.append((x0 << 4) | x1)

    return data


def rgb12(im):
    pix = im.load()
    width, height = im.size
    data = array('H')

    for y in range(height):
        for x in range(width):
            r, g, b = pix[x, y]
            data.append(((r & 0xf0) << 4) | (g & 0xf0) | (b >> 4))

    return data


def do_bitmap(im, desc):
    if im.mode not in ['1', 'L', 'P']:
        raise SystemExit('Only 8-bit images supported.')

    param = parse(desc,
                  ('name', str),
                  ('width,height,depth', (int, int, int)),
                  ('extract_at', (int, int), (0, 0)),
                  ('interleaved', bool, False),
                  ('displayable', bool, True),
                  ('shared', bool, False),
                  ('limit_depth', bool, False),
                  ('onlydata', bool, False))

    name = param['name']
    has_width = param['width']
    has_height = param['height']
    has_depth = param['depth']
    x, y = param['extract_at']
    interleaved = param['interleaved']
    displayable = param['displayable']
    shared = param['shared']
    limit_depth = param['limit_depth']
    onlydata = param['onlydata']

    w, h = im.size
    im = im.copy().crop((x, y, min(x + has_width, w), min(y + has_height, h)))

    pix = array('B', im.getdata())

    width, height = im.size
    colors = im.getextrema()[1] + 1
    depth = int(ceil(log(colors, 2)))
    if limit_depth:
        depth = min(depth, has_depth)

    if width != has_width or height != has_height or depth != has_depth:
        raise SystemExit(
            'Image is {}, expected {}!'.format(
                'x'.join(map(str, [width, height, depth])),
                'x'.join(map(str, [has_width, has_height, has_depth]))))

    bytesPerRow = ((width + 15) & ~15) // 8
    wordsPerRow = bytesPerRow // 2
    bplSize = bytesPerRow * height
    bpl = planar(pix, width, height, depth)

    data_chip = '__data_chip' if displayable else ''

    print(f'static {data_chip} u_short _{name}_bpl[] = {{')
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

    print(f'#define {name}_width {width}')
    print(f'#define {name}_height {height}')
    print(f'#define {name}_depth {depth}')
    print(f'#define {name}_bytesPerRow {bytesPerRow}')
    print(f'#define {name}_bplSize {bplSize}')
    print(f'#define {name}_size {bplSize*depth}')
    print('')

    if onlydata:
        return

    print('%sconst __data BitmapT %s = {' % ('' if shared else 'static ', name))
    print(f'  .width = {width},')
    print(f'  .height = {height},')
    print(f'  .depth = {depth},')
    print(f'  .bytesPerRow = {bytesPerRow},')
    print(f'  .bplSize = {bplSize},')
    flags = ['BM_STATIC']
    if displayable:
        flags.append('BM_DISPLAYABLE')
    if interleaved:
        flags.append('BM_INTERLEAVED')
    print('  .flags = %s,' % '|'.join(flags))
    print('  .planes = {')
    for i in range(depth):
        if interleaved:
            offset = i * bytesPerRow
        else:
            offset = i * bplSize
        print(f'    (void *)_{name}_bpl + {offset},')
    print('  }')
    print('};')


def do_sprite(im, desc):
    if im.mode not in ['1', 'L', 'P']:
        raise SystemExit('Only 8-bit images supported.')

    param = parse(desc,
                  ('name', str),
                  ('height', int),
                  ('count', int),
                  ('attached', bool, False))

    name = param['name']
    has_height = param['height']
    has_count = param['count']
    attached = param['attached']

    pix = array('B', im.getdata())

    width, height = im.size
    colors = im.getextrema()[1] + 1
    depth = int(ceil(log(colors, 2)))

    if height != has_height:
        raise SystemExit(
            f'Image height is {height}, expected {has_height}!')

    exp_width = has_count * 16
    if width != exp_width:
        raise SystemExit(
            f'Image width is {width}, expected {exp_width}!')

    if not attached and depth != 2:
        raise SystemExit(f'Image depth is {depth}, expected 2!')

    if attached and depth != 4:
        raise SystemExit(f'Image depth is {depth}, expected 4!')

    stride = ((width + 15) & ~15) // 16
    bpl = planar(pix, width, height, depth)

    print(f'static const short {name}_height = {height};')
    print('')

    n = width // 16
    if attached:
        n *= 2

    sprites = []

    for i in range(n):
        sprite = name
        if width > 16:
            sprite += str(i)

        attached_sprite = attached and i % 2 == 1

        offset = stride * 2 if attached_sprite else 0
        offset += i // 2 if attached else i

        attached_str = str(attached_sprite).lower()

        print(f'static __data_chip SprDataT {sprite}_sprdat = {{')
        print(f'  .pos = SPRPOS(0, 0),')
        print(f'  .ctl = SPRCTL(0, 0, {attached_str}, {height}),')
        print('  .data = {')
        for j in range(0, stride * depth * height, stride * depth):
            words = bpl[offset + j], bpl[offset + j + stride]
            print('    { 0x%04x, 0x%04x },' % words)
        print('    /* sprite channel terminator */')
        print('    { 0x0000, 0x0000 },')
        print('  }')
        print('};')
        print('')

        sprites.append((sprite, attached_str))

    if n > 1:
        print(f'static __data SpriteT {name}[{n}] = {{')
        for sprite, attached_str in sprites:
            print('  {')
            print(f'    .sprdat = &{sprite}_sprdat,')
            print(f'    .height = {height},')
            print(f'    .attached = {attached_str},')
            print('  },')
        print('};')
    else:
        print(f'static __data SpriteT {name} = {{')
        print(f'  .sprdat = &{name}_sprdat,')
        print(f'  .height = {height},')
        print(f'  .attached = false,')
        print('};')


def do_pixmap(im, desc):
    param = parse(desc,
                  ('name', str),
                  ('width,height,bpp', (int, int, int)),
                  ('onlydata', bool, False))

    name = param['name']
    has_width = param['width']
    has_height = param['height']
    has_bpp = param['bpp']
    onlydata = param['onlydata']

    width, height = im.size

    if width != has_width or height != has_height:
        raise SystemExit('Image size is %dx%d, expected %dx%d!' % (
            width, height, has_width, has_height))

    if has_bpp not in [4, 8, 12]:
        raise SystemExit('Wrong specification: bits per pixel %d!' % has_bpp)

    stride = width
    pixeltype = None
    data = None

    if im.mode in ['1', 'L', 'P']:
        if has_bpp > 8:
            raise SystemExit('Expected grayscale / color mapped image!')

        colors = im.getextrema()[1] + 1
        bpp = int(ceil(log(colors, 2)))
        if bpp > has_bpp:
            raise SystemExit(
                'Image\'s bits per pixel is %d, expected %d!' % (bpp, has_bpp))

        if has_bpp == 4:
            pixeltype = 'PM_CMAP4'
            data = chunky4(im)
            stride = (width + 1) // 2
        else:
            pixeltype = 'PM_CMAP8'
            data = array('B', im.getdata())

        print('static u_char %s_pixels[%d] = {' % (name, stride * height))
        for i in range(0, stride * height, stride):
            row = ['0x%02x' % p for p in data[i:i + stride]]
            print('  %s,' % ', '.join(row))
        print('};')
        print('')
    elif im.mode in ['RGB']:
        if has_bpp <= 8:
            raise SystemExit('Expected RGB true color image!')
        pixeltype = 'PM_RGB12'
        data = rgb12(im)

        print('static u_short %s_pixels[%d] = {' % (name, stride * height))
        for i in range(0, stride * height, stride):
            row = ['0x%04x' % p for p in data[i:i + stride]]
            print('  %s,' % ', '.join(row))
        print('};')
        print('')
    else:
        raise SystemExit('Image pixel format %s not handled!' % im.mode)

    print('#define %s_width %d' % (name, width))
    print('#define %s_height %d' % (name, height))
    print('')

    if not onlydata:
        print('static const __data PixmapT %s = {' % name)
        print('  .type = %s,' % pixeltype)
        print('  .width = %d,' % width)
        print('  .height = %d,' % height)
        print('  .pixels = %s_pixels' % name)
        print('};')
        print('')


def do_palette(im, desc):
    if im.mode != 'P':
        raise SystemExit('Only 8-bit images with palette supported.')

    param = parse(desc,
                  ('name', str),
                  ('colors', int),
                  ('shared', bool, False),
                  ('store_unused', bool, False))

    name = param['name']
    has_colors = param['colors']
    shared = param['shared']
    store_unused = param['store_unused']

    pal = im.getpalette()
    colors = im.getextrema()[1] + 1

    if pal is None:
        raise SystemExit('Image has no palette!')
    if colors > has_colors:
        raise SystemExit('Image has {} colors, expected at most {}!'
                         .format(colors, has_colors))

    if store_unused:
        colors = max(colors, has_colors)

    cmap = [pal[i * 3:(i + 1) * 3] for i in range(colors)]

    print("#define %s_count %d\n" % (name, len(cmap)))

    print('%sconst __data PaletteT %s = {' % ('' if shared else 'static ', name))
    print('  .count = %d,' % len(cmap))
    print('  .colors = {')
    for r, g, b in cmap:
        print('    0x%x%x%x,' % (r >> 4, g >> 4, b >> 4))
    print('  }')
    print('};')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Converts an image to bitmap.')
    parser.add_argument('--bitmap', type=str, action='append',
                        help='Output Amiga bitmap [name,dimensions,flags]')
    parser.add_argument('--pixmap', type=str,
                        help='Output pixel map '
                             '[name,width,height,type,flags]')
    parser.add_argument('--sprite', type=str,
                        help='Output Amiga sprite [name]')
    parser.add_argument('--palette', type=str,
                        help='Output Amiga palette [name,colors]')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='Input image filename')
    args = parser.parse_args()

    im = Image.open(args.path)

    if args.palette:
        do_palette(im, args.palette)
        print('')

    if args.bitmap:
        for bm in args.bitmap:
            do_bitmap(im, bm)
        print('')

    if args.pixmap:
        do_pixmap(im, args.pixmap)
        print('')

    if args.sprite:
        do_sprite(im, args.sprite)
        print('')
