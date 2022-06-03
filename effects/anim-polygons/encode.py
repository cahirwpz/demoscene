#!/usr/bin/env python3

import sys
import os.path
import argparse
from textwrap import TextWrapper
import xml.etree.ElementTree as ET

XMLNS = {"svg": "http://www.w3.org/2000/svg"}


def get_coords(letter, prev_coords, current_coords):
    if letter == "M" or letter == "L":
        return (current_coords[0], current_coords[1])
    if letter == "m" or letter == "l":
        x = prev_coords[0] + current_coords[0]
        y = prev_coords[1] + current_coords[1]
        return (x, y)
    if letter == "H":
        x = current_coords[0]
        y = prev_coords[1]
        return (x, y)
    if letter == "h":
        x = prev_coords[0] + current_coords[0]
        y = prev_coords[1]
        return (x, y)
    if letter == "V":
        x = prev_coords[0]
        y = current_coords[0]
        return (x, y)
    if letter == "v":
        x = prev_coords[0]
        y = prev_coords[1] + current_coords[0]
        return (x, y)
    sys.exit(f"Unknown letter in path element: {letter}")


def parse_numbers(frame_path, it):
    coords = [0, 0]
    i = it
    j = it + 1
    for xy in range(2):
        while (frame_path[j].isdecimal() or frame_path[j] in "-.") \
               and j < len(frame_path) - 1:
            j += 1
        coords[xy] = int(float(frame_path[i:j]))
        while frame_path[j].isspace():
            j += 1
        if frame_path[j] != ",":
            break
        i = j + 1
        j += 2
    return (coords[0], coords[1]), j - 1


def parse_frame(frame_path, polys):
    verts = []
    frame_path += " "
    letter = ""
    current_coords = (0, 0)
    prev_coords = (0, 0)

    i = -1
    while i < len(frame_path) - 1:
        i += 1
        if frame_path[i].isspace():
            continue
        if frame_path[i].isalpha():
            letter = frame_path[i]
            if letter.upper() == "Z":
                polys.append(verts)
                verts = []
                prev_coords = (0, 0)
            continue
        if frame_path[i].isdecimal() or frame_path[i] == "-":
            current_coords, i = parse_numbers(frame_path, i)
            prev_coords = get_coords(letter, prev_coords, current_coords)
            verts.append(prev_coords)


def read_anim(path):
    ET.register_namespace('', XMLNS["svg"])
    tree = ET.parse(path)
    svg_element = tree.getroot()
    width = svg_element.attrib["width"]
    height = svg_element.attrib["height"]

    anim = svg_element[0]
    for tag in svg_element:
        if tag.attrib["id"] == "Animation":
            anim = tag
            break

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
