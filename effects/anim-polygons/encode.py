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


def parse_path(path, color_index, polys):
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
            polys.append([color_index, verts])
            verts = []
        else:
            raise ValueError(f"Unknown drawto command '{cmd}'!")


def torgb(c: str) -> tuple[int, int, int]:
    r = int(c[1:3], 16)
    g = int(c[3:5], 16)
    b = int(c[5:7], 16)
    return (r, g, b)


def rgb12(color: str) -> str:
    r, g, b = torgb(color)
    r = (r + 7) >> 4
    g = (g + 7) >> 4
    b = (b + 7) >> 4
    return f"{r:x}{g:x}{b:x}"


def read_anim(path, background):
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
        colors = []
        if background is not None:
            colors.append(background)
        for element in frame[0]:
            color = element.attrib.get('fill', '#000000')
            if color not in colors:
                colors.append(color)

        polys = []
        for element in frame[0]:
            color = element.attrib.get('fill', '#000000')
            if color == background:
                continue
            color_index = colors.index(color)
            path = element.attrib["d"]
            parse_path(path, color_index, polys)

        frames.append([[rgb12(color) for color in colors], polys])

    return frames, int(width), int(height)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Converts .svg into .c animation file for "
                    "anim-polygons effect.")
    parser.add_argument(
        "-c", "--has-colors", action="store_true",
        help="Enable color SVG handling.")
    parser.add_argument(
        "-b", "--background", type=str,
        help="Remove all polygons with this color and set it as background "
             "(format #RRGGBB)")
    parser.add_argument(
        "animation",
        help="Path to .svg file generated using pipeline described "
             "in prototypes/anim_polygons_data/ directory.",
        type=str)
    args = parser.parse_args()
    frames, width, height = read_anim(args.animation, args.background)
    name, _ = os.path.splitext(os.path.basename(args.animation))

    wrapper = TextWrapper(initial_indent=' ' * 2,
                          subsequent_indent=' ' * 4,
                          width=80)

    for num, (colors, polys) in enumerate(frames):
        count = sum(len(poly) for poly in polys) + 1

        print('static short %s_frame%d[] = {' % (name, num))

        if args.has_colors:
            colors = [f'0x{c[1:]:03}' for c in colors]
            print('  0x%04x, %s,' % (len(colors), ', '.join(colors)))

        for color, poly in polys:
            if not args.has_colors:
                color = 0
            code = (color << 8) | len(poly)
            verts = [f'0x{code:04x}']
            for x, y in poly:
                x = min(max(0, x), width - 1)
                y = min(max(0, y), height - 1)
                verts.append(str(y * width + x))
            for line in wrapper.wrap(', '.join(verts) + ','):
                print(line)

        print('  0,')
        print('};')
        print('')

    print('#define %s_frames %d' % (name, len(frames)))
    print('')
    print('static short *%s_frame[] = {' % name)
    for num, _ in enumerate(frames):
        print('  %s_frame%d,' % (name, num))
    print('};')
