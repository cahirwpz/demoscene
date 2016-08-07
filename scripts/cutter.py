#!/usr/bin/env python

import argparse
import Image
import json
import subprocess

import any2ilbm


def limit(val, base, colors):
    val = max(0, val - base)
    val = min(val, colors - 1)
    return val

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Extract rectangular areas from an image.')
    parser.add_argument(
        'input', metavar='INPUT', type=str,
        help='Path to JSON file which describes action to perform.')
    args = parser.parse_args()

    with open(args.input) as f:
        data = json.load(f)
        src = Image.open(data['input'])

        for path, ctrl in data['output'].items():
            x, y, w, h = ctrl['area']
            colors = ctrl.get('colors', data['colors'])
            base = ctrl.get('base-color', 0)

            im = src.crop((x, y, x + w, y + h))
            pix = [limit(p, base, colors) for p in im.getdata()]
            pal = src.getpalette()[base * 3:(base + colors) * 3]
            pal.extend([0 for i in range(768 - len(pal))])

            dst = Image.new('P', (w, h))
            dst.putdata(pix)
            dst.putpalette(pal)
            dst.save(path + '.png')

            subprocess.call(['optipng', '-silent', path + '.png'])

            any2ilbm.convert(path + '.png', path + '.ilbm')
