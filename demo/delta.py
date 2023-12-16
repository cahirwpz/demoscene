#!/usr/bin/env python3

import argparse
from array import array

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Encode 8-bit samples with delta encoding.')
    parser.add_argument(
        'decoded', metavar='DECODED', type=str, help='Decoded samples file.')
    parser.add_argument(
        'encoded', metavar='ENCODED', type=str, help='Encoded samples file.')
    args = parser.parse_args()

    data = array('B')

    with open(args.decoded, 'rb') as f:
        data.frombytes(f.read())
        data.append(0)

    out = array('B')
    for i in range(len(data) - 1):
        out.append((data[i] - data[i - 1]) & 255)

    with open(args.encoded, 'wb') as f:
        f.write(out)
