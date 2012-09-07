#!/usr/bin/env python

from array import array

import Image
import argparse
import logging
import struct
import os

IMG_GRAY  = 0
IMG_CLUT  = 1
IMG_RGB24 = 2

def Main():
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

  imgTypeMap = {'L' : IMG_GRAY, 'P' : IMG_CLUT, 'RGB' : IMG_RGB24}
  imgType = imgTypeMap.get(image.mode, None)

  # Assumes that color #0 is transparency color (for 8-bit images).
  isTransparent = image.info.has_key('transparency')

  if not imgType:
    raise SystemExit('Unknown color space: %s.' % image.mode)

  if imgType is IMG_CLUT:
    palFilePath = '%s.pal' % outputPath
    uniqueColors = len(image.getcolors())

    if isTransparent:
      # Don't forget about transparency color.
      uniqueColors += 1

    if os.path.isfile(palFilePath) and not args.force:
      raise SystemExit('Will not overwrite output file!')

    with open(palFilePath, 'w') as palFile:
      pal = array('B', image.getpalette()[:3*uniqueColors])
      logging.info('Saving palette of %d colors to "%s".',
                   uniqueColors, palFilePath)
      palFile.write(struct.pack('>H', uniqueColors))
      palFile.write(pal.tostring())

  data = array('B')

  if imgType is IMG_RGB24:
    rawFilePath = '%s.24' % outputPath
    uniqueColors = len(set(image.getdata()))

    for rgb in image.getdata():
      data.extend(rgb)
  else:
    rawFilePath = '%s.8' % outputPath
    uniqueColors = len(image.getcolors())

    data.extend(image.getdata())

  width, height = image.size

  if os.path.isfile(rawFilePath) and not args.force:
    raise SystemExit('Will not overwrite output file!')

  with open(rawFilePath, 'w') as rawFile:
    logging.info('Saving image of (%d, %d) size to "%s".',
                 width, height, rawFilePath)
    rawFile.write(struct.pack(
      '>BBHHI', isTransparent, imgType, width, height, uniqueColors))
    rawFile.write(data.tostring())

if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  Main()
