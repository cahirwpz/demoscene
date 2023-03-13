#!/usr/bin/env python3

import array
import argparse
import struct
import os
import sys


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Splits protracker module into patterns and samples.')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='Input protracker filename')
    args = parser.parse_args()

    size = os.path.getsize(args.path)

    with open(args.path, 'rb') as f:
        hdr = f.read(1080)
        if len(hdr) != 1080 or f.read(4) != b'M.K.':
            raise SystemExit('Not a protracker module!')
        song = array.array('B', hdr[-128:])
        patterns = (max(song) + 1) * 1024
        ptlen = f.tell() + patterns

        f.seek(0)
        with open(args.path + '.trk', 'wb') as out:
            out.write(f.read(ptlen))
        with open(args.path + '.smp', 'wb') as out:
            out.write(f.read(size - ptlen))
