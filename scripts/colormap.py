#!/usr/bin/env python

from array import array

import Image
import argparse
import struct
import os

IMG_GRAY  = 0
IMG_CLUT  = 1
IMG_RGB24 = 2

def main():
  parser = argparse.ArgumentParser(
      description='Generates color map required by some pixel effects.')
  parser.add_argument('-f', '--force', action='store_true',
      help='If output files exist, the tool will overwrite them.')
  parser.add_argument('-m', '--map', type=str, default='shades',
      choices=['lighten', 'darken', 'shades', 'transparency'],
      help='Type of color map.')
  parser.add_argument('input', metavar='INPUT', type=str,
      help='Input image filename.')
  parser.add_argument('output', metavar='OUTPUT', type=str,
      help='Output files basename (without extension).')
  args = parser.parse_args()

  inputPath = os.path.abspath(args.input)
  outputPath = os.path.abspath(args.output) + '.png'

  if not os.path.isfile(inputPath):
    raise SystemExit('Input file does not exists!')

  try:
    image = Image.open(inputPath)
  except IOError as ex:
    raise SystemExit('Error: %s.' % ex)

  if image.mode != 'P':
    raise SystemExit('Image has to be of CLUT type.')

  rawPal = image.getpalette() 
  colors = len(set(image.getdata()))
  pal = sorted([tuple(rawPal[3*i:3*(i+1)]) for i in range(colors)])

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
  colorMap = colorMap._new(colorMap.im.convert('P', 0, image.im))

  if os.path.isfile(outputPath) and not args.force:
    raise SystemExit('Will not overwrite output file!')

  colorMap.save(outputPath)

if __name__ == '__main__':
  main()
