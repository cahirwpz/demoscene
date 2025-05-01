#!/usr/bin/env python3

from PIL import Image
import sys


def loadPalette(imgPath):
    im = Image.open(imgPath)
    pixels = list(im.getdata())
    width, height = im.size
    pixels = [pixels[i * width:(i + 1) * width] for i in range(height)]

    out = []
    for p in pixels:
        v = (p[0][0] & 0xF0) << 4
        v += (p[0][1] & 0xF0)
        v += (p[0][2] & 0xF0) >> 4
        out.append(v)

    return out


def scramblePixels(c):
    '''
    For given RGB12 value returns color in format
    [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3]
    '''
    r0 = '1' if c & 0b000100000000 else '0'  # RED
    r1 = '1' if c & 0b001000000000 else '0'
    r2 = '1' if c & 0b010000000000 else '0'
    r3 = '1' if c & 0b100000000000 else '0'
    g0 = '1' if c & 0b000000010000 else '0'  # GREEN
    g1 = '1' if c & 0b000000100000 else '0'
    g2 = '1' if c & 0b000001000000 else '0'
    g3 = '1' if c & 0b000010000000 else '0'
    b0 = '1' if c & 0b000000000001 else '0'  # BLUE
    b1 = '1' if c & 0b000000000010 else '0'
    b2 = '1' if c & 0b000000000100 else '0'
    b3 = '1' if c & 0b000000001000 else '0'
    out0 = r0+g0+b0+b0
    out1 = r1+g1+b1+b1
    out2 = r2+g2+b2+b2
    out3 = r3+g3+b3+b3
    return int(out3+out2+out1+out0, 2)


if __name__ == '__main__':
    '''
    Generates table of 32bit values where the first 16 bits word equals
    the value of (index * 63 / 256) and the second 16 bits word represents
    color coresponding to this value.
    Values 63 and 256 can be adjusted to make fire bigger/smaller.
    '''
    img_path = sys.argv[1]
    palette = loadPalette(img_path)

    print('static uint32_t dualtab[256] = {')
    for i in range(256):
        val = int(i * 63 / 256)
        scrambledColor = scramblePixels(palette[val])
        print('  ' + hex((val << 16) * 4 + scrambledColor) + ',')
    print('};')
