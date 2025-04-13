#!/usr/bin/env python3

from array import array
import argparse
import os


def delta(rawdata):
    data = array('B')
    data.frombytes(rawdata)
    data.append(0)

    out = array('B')
    for i in range(len(data) - 1):
        out.append((data[i] - data[i - 1]) & 255)

    return out.tobytes()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Splits protracker module into patterns and samples.')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='Input protracker filename')
    parser.add_argument('--delta', action='store_true',
                        help='Perform delta encoding for better compression')
    parser.add_argument('--extract', choices=['trk', 'smp'],
                        help='Extract either pattern data or samples')
    parser.add_argument('-o', '--output', metavar='OUTPUT', type=str,
                        help='Output file path')
    args = parser.parse_args()

    size = os.path.getsize(args.path)

    with open(args.path, 'rb') as f:
        hdr = f.read(1080)
        if len(hdr) != 1080 or f.read(4) != b'M.K.':
            raise SystemExit('Not a protracker module!')
        song = array('B', hdr[-128:])
        patterns = (max(song) + 1) * 1024
        ptlen = f.tell() + patterns

        if args.extract == 'trk':
            f.seek(0)
            with open(args.output, 'wb') as out:
                out.write(f.read(ptlen))
        elif args.extract == 'smp':
            f.seek(ptlen)
            with open(args.output, 'wb') as out:
                data = f.read(size - ptlen)
                if args.delta:
                    data = delta(data)
                out.write(data)
