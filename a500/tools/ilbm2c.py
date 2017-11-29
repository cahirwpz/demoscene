#!/usr/bin/env python

from __future__ import print_function

import argparse
import logging
import sys
import zopfli

from iff.ilbm import ILBM, UnRLE, Deinterleave
from contextlib import contextmanager


@contextmanager
def redirect_stdout(fileobj):
    old_stdout = sys.stdout
    sys.stdout = fileobj
    try:
        yield fileobj
    finally:
        sys.stdout = old_stdout


def main(path, name, args):
    ilbm = ILBM()
    ilbm.ChunkBlackList.append('JUNK')
    ilbm.load(path)

    with open(name + '.c', 'w') as f:
        bmhd = ilbm.get('BMHD').data
        cmap = ilbm.get('CMAP').data
        body = ilbm.get('BODY').data.getvalue()

        with redirect_stdout(f):
            bytesPerRow = bmhd.w / 8
            bplSize = bmhd.h * bmhd.w / 8
            size = ((bmhd.w + 15) & ~15) / 8 * bmhd.h * bmhd.nPlanes

            if bmhd.compression in [0, 1, 254]:
                if bmhd.compression == 1:
                    body = UnRLE(body)
                if bmhd.compression == 254:
                    body = zopfli.decompress(body, size)
            else:
              logging.warning('Unknown compression: %d' % bmhd.compression)

            if args.deinterleave:
                body = Deinterleave(body, bmhd.w, bmhd.h, bmhd.nPlanes)

            # file header
            if args.only in ['all', 'palette']:
                print('#include "gfx.h"')

            # CMAP chunk (palette)
            if not args.no_palette and args.only in ['all', 'palette']:
                print('PaletteT %s_pal = {' % name)
                print('  .count = %d,' % len(cmap))
                print('  .colors = {')
                for i, (r, g, b) in enumerate(cmap):
                    print('    [%d] = {.r = %d, .g = %d, .b = %d},' %
                          (i, r, g, b))
                print('  },')
                print('};')

            # BODY chunk (bitplane data)
            if not args.no_bitplanes and args.only in ['all', 'bitplanes']:
                print('UBYTE %s_data[%d] = {' % (name, len(body)))
                data = map(ord, body)
                for i in range(0, len(data), bytesPerRow):
                    row = data[i:min(len(data), i+bytesPerRow)]
                    bytelist = ['0x%02x' % byte for byte in row]
                    print('  %s,' % ', '.join(bytelist))
                print('};')

            # BMHD chunk (bitmap metadata)
            flags = ['BM_MINIMAL']
            if not args.deinterleave:
                flags.append('BM_INTERLEAVED')

            if args.only is 'all':
                print('BitmapT %s = {' % name)
                print('  .width = %d,' % bmhd.w)
                print('  .height = %d,' % bmhd.h)
                print('  .depth = %d,' % bmhd.nPlanes)
                print('  .bytesPerRow = %d,' % bytesPerRow)
                print('  .bplSize = %d,' % bplSize)
                print('  .flags = %s,' % '|'.join(flags))
                print('  .compression = COMP_NONE,')

                if args.no_palette:
                    print('  .palette = NULL,')
                else:
                    print('  .palette = &%s_pal,' % name)

                print('  .planes = {')
                if args.no_bitplanes:
                    for i in range(bmhd.nPlanes):
                        print('    [%d] = NULL,' % i)
                else:
                    space = bplSize if args.deinterleave else bytesPerRow
                    for i in range(bmhd.nPlanes):
                        print('    [%d] = &%s_data[%d],' % (i, name, i * space))
                print('  },')
                print('};')


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

    parser = argparse.ArgumentParser(
        description='Dumps ILBM image as C structure.')
    parser.add_argument('--deinterleave', action='store_true',
                        help='Deinterleave bitplanes.')
    parser.add_argument('--no-bitplanes', action='store_true',
                        help='Do not dump bitplane data.')
    parser.add_argument('--no-palette', action='store_true',
                        help='Do not dump palette data.')
    parser.add_argument('--only', choices=['bitplanes', 'palette', 'all'],
                        default='all',
                        help='Dump only bitplane or palette data.')
    parser.add_argument('input', metavar='INPUT', type=str,
                        help='Input image filename.')
    parser.add_argument('output', metavar='OUTPUT', type=str,
                        help='Input image filename.')
    args = parser.parse_args()

    assert args.output is not None

    main(args.input, args.output, args)
