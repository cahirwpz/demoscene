#!/usr/bin/env python

import Image
import argparse
import os
import json

from quantize import Quantize

IMG_GRAY = 0
IMG_CLUT = 1
IMG_RGB24 = 2


if __name__ == '__main__':
  parser = argparse.ArgumentParser(
    description='Generate color map required by some pixel effects.')
  parser.add_argument(
    '-c', '--colors', type=int, default=None,
    help=('Generate new palette with given number of colors. '
          'Otherwise use original palette.'))
  parser.add_argument(
    '-f', '--force', action='store_true',
    help='If output files exist, the tool will overwrite them.')
  parser.add_argument(
    '-m', '--map', type=str, default='shades',
    choices=['lighten', 'darken', 'shades', 'transparency'],
    help='Type of color map.')
  parser.add_argument(
    'input', metavar='INPUT', type=str,
    help='Path to input image or JSON (with array of colors) file.')
  parser.add_argument(
    'output', metavar='OUTPUT', type=str,
    help='Output files basename (without extension).')
  args = parser.parse_args()

  inputPath = os.path.abspath(args.input)
  outputPath = os.path.abspath(args.output)

  if not os.path.isfile(inputPath):
    raise SystemExit('Input file does not exists!')

  if inputPath.endswith('.json'):
    rawPal = []
    pal = []

    with open(inputPath) as f:
      for color in json.load(f):
        assert len(color) == 3
        color = int(color[0]), int(color[1]), int(color[2])
        assert all(comp >= 0 and comp <= 255 for comp in color)
        pal.append(color)
        rawPal.extend(color)

    colors = len(pal)
  else:
    try:
      image = Image.open(inputPath)
    except IOError as ex:
      raise SystemExit('Error: %s.' % ex)

    if image.mode != 'P':
      raise SystemExit('Image has to be of CLUT type.')

    rawPal = image.getpalette()
    colors = len(set(image.getdata()))
    pal = [tuple(rawPal[3 * i:3 * (i + 1)]) for i in range(colors)]

  colorMapData = []

  if args.map == 'lighten':
    size = (256, colors)

    for y in range(size[1]):
      for x in range(size[0]):
        r, g, b = pal[y][0], pal[y][1], pal[y][2]
        a = float(x) / 255.0
        r += (255 - r) * a
        g += (255 - g) * a
        b += (255 - b) * a
        colorMapData.append((int(r), int(g), int(b)))

  elif args.map == 'darken':
    size = (256, colors)

    for y in range(size[1]):
      for x in range(size[0]):
        r, g, b = pal[y][0], pal[y][1], pal[y][2]
        a = 1.0 - float(x) / 255.0
        r -= r * a
        g -= g * a
        b -= b * a
        colorMapData.append((int(r), int(g), int(b)))

  elif args.map == 'shades':
    size = (256, colors)

    for y in range(size[1]):
      for x in range(size[0]):
        r, g, b = pal[y][0], pal[y][1], pal[y][2]
        a = 2.0 * float(x) / 255.0 - 1.0
        if a >= 0.0:
          r += (255 - r) * a
          g += (255 - g) * a
          b += (255 - b) * a
        else:
          r += r * a
          g += g * a
          b += b * a
        colorMapData.append((int(r), int(g), int(b)))

  elif args.map == 'transparency':
    size = (colors, colors)

    for y in range(colors):
      for x in range(colors):
        r, g, b = pal[y][0], pal[y][1], pal[y][2]
        r = (r + pal[x][0]) / 2
        g = (g + pal[x][1]) / 2
        b = (b + pal[x][2]) / 2
        colorMapData.append((int(r), int(g), int(b)))

  colorMap = Image.new('RGB', size)
  colorMap.putdata(colorMapData)

  if not args.colors:
    data = []

    for color in colorMapData:
      pixel = 0
      dist = 3 * 255 * 255
      r, g, b = color

      for i, neighbour in enumerate(pal):
        nr, ng, nb = neighbour

        nr = r - nr
        ng = g - ng
        nb = b - nb

        d = nr * nr + ng * ng + nb * nb

        if d < dist:
          dist = d
          pixel = i

      data.append(pixel)

    output = Image.new('L', size)
    output.putdata(data)
    output.putpalette(rawPal)
  else:
    output = Quantize(colorMap, colors=args.colors, dithering=False)

  if os.path.isfile(outputPath) and not args.force:
    raise SystemExit('Will not overwrite output file!')

  output.save(outputPath)
