#!/usr/bin/env python3

from sys import argv
from PIL import Image
import re
import argparse
 
header_pattern = re.compile(
    r'x\s*=\s*(\d+)\s*,\s*y\s*=\s*(\d+)(?:\s*,\s*rule\s*=\s*(\w+\/\w+)\s*)*'
)
rle_pattern = re.compile(r'(\d*)(o|b|\$)')
rotate_transforms = {
    90: Image.ROTATE_90,
    180: Image.ROTATE_180,
    270: Image.ROTATE_270,
}

def convert_rle(infile):
    with open(infile, 'r') as f:
        # skip comments
        line = f.readline().strip()
        while line.startswith("#"):
            line = f.readline().strip()

        if line == '':
            line = f.readline().strip()
        header = line

        # read header
        captures = header_pattern.match(header)
        x = int(captures.group(1))
        y = int(captures.group(2))
        rule = captures.group(3)

        # decode RLE into the image
        # pallete mode (only this can produce 1-bit image,
        # 1 and L seem to always output 8-bit)
        im = Image.new("P", (x, y))
        cur_x = 0
        cur_y = 0
        for line in f:
            captures = [(int(count) if count != '' else 1, tag)
                        for count, tag in rle_pattern.findall(line)]
            for count, tag in captures:
                for i in range(0, count):
                    if tag == 'b':
                        im.putpixel((cur_x, cur_y), 0)
                        cur_x += 1
                    elif tag == 'o':
                        im.putpixel((cur_x, cur_y), 1)
                        cur_x += 1
                    elif tag == '$':
                        cur_y += 1
                        cur_x = 0
                    else:
                        im.putpixel((cur_x, cur_y), 1)
                        cur_x += 1
        im.putpalette([0, 0, 0, 255, 255, 255])
        return im

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Converts RLE game of life description to 1-bit PNG.")
    parser.add_argument("infile", help="Input RLE file")
    parser.add_argument("outfile", help="Output PNG file")
    parser.add_argument('--rotate', help="Rotate output image", type=int,
        choices=[90, 180, 270])
    args = parser.parse_args()

    im = convert_rle(args.infile)

    if args.rotate:
        im = im.transpose(rotate_transforms[args.rotate])

    im.save(args.outfile, bits=1)
