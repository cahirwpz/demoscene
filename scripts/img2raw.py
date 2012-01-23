#!/usr/bin/env python

import Image
import argparse
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

  rawFilePath = '%s.raw' % outputPath
  palFilePath = '%s.pal' % outputPath

  if not os.path.isfile(inputPath):
    raise SystemExit('Input file does not exists!')

  if any(map(os.path.isfile, [rawFilePath, palFilePath])) and not args.force:
    raise SystemExit('Will not overwrite output files!')

  try:
    image = Image.open(inputPath)
  except IOError as ex:
    raise SystemExit('Error: %s.' % ex)
  else:
    with open(palFilePath, 'w') as palFile:
      pal = map(chr, image.getpalette())
      palFile.write("".join(pal))

    with open(rawFilePath, 'w') as rawFile:
      w, h = image.size
      for y in range(h):
        for x in range(w):
          pixel = image.getpixel((x,y))
          rawFile.write(chr(pixel))

if __name__ == '__main__':
  main()
