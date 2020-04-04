#!/usr/bin/env python3

import argparse
import os.path
from shlex import split


def convert(lines, name):
    section = []

    origin = (0, 0)
    points = []
    npoints = 0
    polygons = []
    npolygons = 0

    for line in lines:
        fs = split(line, comments=True)
        if not len(fs):
            continue

        if not section:
            if fs[0] == '@origin':
                origin = tuple(map(int, fs[1:]))
            elif fs[0] == '@pnts':
                npoints = int(fs[1])
                section.append(fs[0])
            elif fs[0] == '@pols':
                npolygons = int(fs[1])
                section.append(fs[0])
            else:
                raise ValueError('Unknown section:', fs[0])
        elif section[-1] == '@pnts':
            if fs[0] == '@end':
                section.pop()
                assert len(points) == npoints
            else:
                assert len(fs) == 2
                points.append(tuple(map(int, fs)))
        elif section[-1] == '@pols':
            if fs[0] == '@end':
                section.pop()
                assert len(polygons) == npolygons
            else:
                assert len(fs) == 2
                polygons.append(list(map(int, fs)))

    print('static Point2D _%s_points[%d] = {' % (name, npoints))
    for x, y in points:
        x = (x - origin[0]) * 16
        y = (y - origin[1]) * 16
        print('  {.x = %4d, .y = %4d},' % (x, y))
    print('};')
    print()

    print('static Point2D _%s_view_points[%d];' % (name, npoints))
    print('static u_char _%s_view_flags[%d];' % (name, npoints))
    print('')

    print('static short _%s_indices[] = {' % name)
    v = 0
    for _, n in polygons:
        indices = ', '.join(map(str, list(range(v, v + n)) + [v]))
        print('    %d, %s,' % (n, indices))
        v += n
    print('};')

    print('static IndexListT *_%s_polygon[%d] = {' % (name, npolygons))
    i = 0
    for _, n in polygons:
        print('    (IndexListT *)&_%s_indices[%d],' % (name, i))
        i += n + 2
    print('};')
    print()

    print('static u_char _%s_polygon_flags[%d] = {' % (name, npolygons))
    for f, _ in polygons:
        print('  %d,' % f)
    print('};')
    print()

    print('ShapeT %s = {' % name)
    print('  .points = %d,' % npoints)
    print('  .polygons = %d,' % npolygons)
    print('  .origPoint = _%s_points,' % name)
    print('  .viewPoint = _%s_view_points,' % name)
    print('  .viewPointFlags = _%s_view_flags,' % name)
    print('  .polygon = _%s_polygon,' % name)
    print('  .polygonFlags = _%s_polygon_flags' % name)
    print('};')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Convert 2D shape file to C representation.')
    parser.add_argument('--name', metavar='NAME', type=str,
                        help='Base name of C objects.')
    parser.add_argument('path', metavar='PATH', type=str,
                        help='2D shape file.')
    args = parser.parse_args()

    if not os.path.isfile(args.path):
        raise SystemExit('Input file does not exists!')

    with open(args.path) as f:
        convert(f, args.name)
