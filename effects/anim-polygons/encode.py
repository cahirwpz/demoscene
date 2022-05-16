#!/usr/bin/env python3

import sys
import os.path
import argparse
from textwrap import TextWrapper
import xml.etree.ElementTree as ET

xmlns = {"svg": "http://www.w3.org/2000/svg"}
OFFSET = 10


def get_coords(code):
    xy = code.split(",")
    x = int(xy[0]) - OFFSET
    y = int(xy[1]) - OFFSET
    return (x, y)


def parse_frame(frame_path):
    first = True
    polys = []
    verts = []
    for code in frame_path.split():
        if code[0] == "Z":
            if len(verts) >= 3:
                if first:
                    first = False
                else:
                    polys.append(verts)
            verts = []
        else:
            verts.append(get_coords(code[1:]))
    return polys


def read_anim(path):
    ET.register_namespace('', xmlns["svg"])
    tree = ET.parse(path)
    anim = tree.getroot()

    frames = []
    for svg_path in anim:
        frames.append(parse_frame(svg_path.attrib["d"]))
    return frames


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="Converts .svg into "
                                                 ".c animation file "
                                                 "for anim-polygons effect.")
    parser.add_argument("animation", help="Path to .svg file "
                                          "generated using pipeline described "
                                          "in prototypes/anim_polygons_data/ "
                                          "directory.", type=str)
    args = parser.parse_args()
    path = args.animation
    frames = read_anim(path)
    name, _ = os.path.splitext(os.path.basename(path))

    Wrapper = TextWrapper(initial_indent=' ' * 2,
                          subsequent_indent=' ' * 4,
                          width=80)

    width = 320
    height = 180

    for fn, polys in enumerate(frames):
        count = sum(len(poly) for poly in polys) + 1

        print('static short %s_frame%d[] = {' % (name, fn))

        for poly in polys:
            verts = [len(poly)]
            for x, y in poly:
                x = min(max(0, x), width - 1)
                y = min(max(0, y), height - 1)
                verts.append(y * width + x)
            for line in Wrapper.wrap(', '.join(map(str, verts)) + ','):
                print(line)

        print('  0,')
        print('};')
        print('')

    print('#define %s_frames %d' % (name, len(frames)))
    print('')
    print('static short *%s_frame[] = {' % name)
    for fn, _ in enumerate(frames):
        print('  %s_frame%d,' % (name, fn))
    print('};')
