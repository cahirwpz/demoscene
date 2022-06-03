#!/usr/bin/env python3

import os.path
import argparse
from textwrap import TextWrapper
import xml.etree.ElementTree as ET

XMLNS = {"svg": "http://www.w3.org/2000/svg"}


def get_coords(code):
    xy = code.split(",")
    x = int(xy[0])
    y = int(xy[1])
    return (x, y)


def parse_frame(frame_path, polys):
    verts = []
    for code in frame_path.split():
        if code[0] == "Z":
            polys.append(verts)
            verts = []
        else:
            verts.append(get_coords(code[1:]))


def read_anim(path):
    ET.register_namespace('', XMLNS["svg"])
    tree = ET.parse(path)
    svg_element = tree.getroot()
    width = svg_element.attrib["width"]
    height = svg_element.attrib["height"]
    anim = svg_element[0]

    frames = []
    for frame in anim:
        polys = []
        for element in frame[0]:
            parse_frame(element.attrib["d"], polys)
        frames.append(polys)

    return frames, int(width), int(height)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
            description="Converts .svg into .c animation file for "
                        "anim-polygons effect.")
    parser.add_argument(
            "animation",
            help="Path to .svg file generated using pipeline described "
                 "in prototypes/anim_polygons_data/ directory.",
            type=str)
    args = parser.parse_args()
    frames, width, height = read_anim(args.animation)
    name, _ = os.path.splitext(os.path.basename(args.animation))

    wrapper = TextWrapper(initial_indent=' ' * 2,
                          subsequent_indent=' ' * 4,
                          width=80)

    for fn, polys in enumerate(frames):
        count = sum(len(poly) for poly in polys) + 1

        print('static short %s_frame%d[] = {' % (name, fn))

        for poly in polys:
            verts = [len(poly)]
            for x, y in poly:
                x = min(max(0, x), width - 1)
                y = min(max(0, y), height - 1)
                verts.append(y * width + x)
            for line in wrapper.wrap(', '.join(map(str, verts)) + ','):
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
