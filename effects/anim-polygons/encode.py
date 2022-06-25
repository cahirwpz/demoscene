#!/usr/bin/env python3

import argparse
import os.path
import re
from textwrap import TextWrapper
import xml.etree.ElementTree as ET

XMLNS = {"svg": "http://www.w3.org/2000/svg"}

# Grammar of SVG path is described here:
# https://www.w3.org/TR/SVG/paths.html#PathDataBNF
# XML EBNF notation is decsribed here:
# https://www.w3.org/TR/REC-xml/#sec-notation


def seq_to_coords(seq, cmd, verts):
    last = verts[-1] if verts else [0, 0]
    for coord in seq:
        if cmd in "ML":
            last = coord
        elif cmd in "ml":
            last = [coord[0] + last[0], coord[1] + last[1]]
        elif cmd == 'H':
            last = [coord, last[1]]
        elif cmd == 'h':
            last = [coord + last[0], last[1]]
        elif cmd == 'V':
            last = [last[0], coord]
        elif cmd == 'v':
            last = [last[0], coord + last[1]]
        else:
            raise ValueError
        verts.append(last)


def parse_wsp(path):
    if m := re.match(r"\s*", path):
        s = m.group(0)
        return path[len(s):]
    raise ValueError("parse_wsp: white space expected!")


def parse_opt_comma_wsp(path):
    # comma_wsp ::= (wsp+ ","? wsp*) | ("," wsp*)
    if m := re.match(r"\s+,?\s*|,\s*", path):
        s = m.group(0)
        return path[len(s):]
    return path


def parse_coord(path):
    # coord ::= sign? number
    if m := re.match(r"[+-]?\d+\.*\d*", path):
        s = m.group(0)
        return path[len(s):], int(float(s))
    raise ValueError("parse_coord: number expected!")


def parse_coord_pair(path):
    # coord_pair ::= coord comma_wsp? coord
    path, x = parse_coord(path)
    path = parse_opt_comma_wsp(path)
    path, y = parse_coord(path)
    return path, [x, y]


def parse_coord_seq(path):
    # coord_seq ::= coord | (coord comma_wsp? coord_seq)
    seq = []
    while re.match(r"[+-]?\d+", path):
        path, x = parse_coord(path)
        seq.append(x)
        path = parse_opt_comma_wsp(path)
    return path, seq


def parse_coord_pair_seq(path):
    # coord_pair_seq ::= coord_pair | (coord_pair comma_wsp? coord_pair_seq)
    seq = []
    while re.match(r"[+-]?\d+", path):
        path, p = parse_coord_pair(path)
        seq.append(p)
        path = parse_opt_comma_wsp(path)
    return path, seq


def parse_path(path, polys):
    verts = []

    while len(path):
        cmd, path = path[0], path[1:]
        if cmd in "MmLl":
            # moveto ::= ("M"|"m") wsp* coordinate_pair_sequence
            # lineto ::= ("L"|"l") wsp* coordinate_pair_sequence
            path = parse_wsp(path)
            path, seq = parse_coord_pair_seq(path)
            seq_to_coords(seq, cmd, verts)
        elif cmd in "HhVv":
            # horizontal_lineto ::= ("H"|"h") wsp* coordinate_sequence
            # vertical_lineto ::= ("V"|"v") wsp* coordinate_sequence
            path = parse_wsp(path)
            path, seq = parse_coord_seq(path)
            seq_to_coords(seq, cmd, verts)
        elif cmd in "Zz":
            # closepath::= ("Z"|"z")
            polys.append(verts)
            verts = []
        else:
            raise ValueError(f"Unknown drawto command '{cmd}'!")


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
            parse_path(element.attrib["d"], polys)
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
