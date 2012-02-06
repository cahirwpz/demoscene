#!/usr/bin/env python

from array import array

import Image
import argparse
import struct
import os

def main():
  parser = argparse.ArgumentParser(
      description='Converts input image to raw image and palette data.')
  parser.add_argument('-f', '--force', action='store_true',
      help='If output files exist, the tool will overwrite them.')
  parser.add_argument('input', metavar='INPUT', type=str,
      help='Input image filename.')
  parser.add_argument('output', metavar='OUTPUT', type=str,
      help='Output files basename (without extension).')
  args = parser.parse_args()

  inputPath = os.path.abspath(args.input)
  outputPath = os.path.abspath(args.output)

  if not os.path.isfile(inputPath):
    raise SystemExit('Input file does not exists!')

  try:
    image = Image.open(inputPath)
  except IOError as ex:
    raise SystemExit('Error: %s.' % ex)

  if image.mode not in ['P', 'RGB']:
    raise SystemExit('Unknown color space: %s.' % image.mode)

  if image.mode == 'P':
    rawFilePath = '%s.8' % outputPath
    palFilePath = '%s.pal' % outputPath

    if any(map(os.path.isfile, [rawFilePath, palFilePath])) and not args.force:
      raise SystemExit('Will not overwrite output files!')

    uniqueColors = len(image.getcolors())
    width, height = image.size

    with open(palFilePath, 'w') as palFile:
      pal = array('B', image.getpalette()[:3*uniqueColors])
      palFile.write(struct.pack('>H', uniqueColors))
      palFile.write(pal.tostring())

    with open(rawFilePath, 'w') as rawFile:
      data = array('B', image.getdata())
      rawFile.write(struct.pack('>HHH', width, height, uniqueColors))
      rawFile.write(data.tostring())
  else:
    rawFilePath = '%s.24' % outputPath

    if os.path.isfile(rawFilePath) and not args.force:
      raise SystemExit('Will not overwrite output file!')

    with open(rawFilePath, 'w') as rawFile:
      data = array('B')

      for rgb in image.getdata():
        data.extend(rgb)

      rawFile.write(struct.pack('>HH', *image.size))
      rawFile.write(data.tostring())

if __name__ == '__main__':
  main()
