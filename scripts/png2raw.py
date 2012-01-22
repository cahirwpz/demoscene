#!/usr/bin/env python

import Image
import logging
import sys

def main(argv):
  pngFileName = sys.argv[1]
  baseFileName, _ = pngFileName.rsplit('.')

  rawFileName = '%s.raw' % baseFileName
  palFileName = '%s.pal' % baseFileName

  image = Image.open(pngFileName)

  with open(palFileName, 'w') as palFile:
    pal = map(chr, image.getpalette())
    palFile.write("".join(pal))

  with open(rawFileName, 'w') as rawFile:
    w, h = image.size
    for y in range(h):
      for x in range(w):
        pixel = image.getpixel((x,y))
        rawFile.write(chr(pixel))

if __name__ == '__main__':
  FORMAT = '%(levelname)s: %(message)s'
  logging.basicConfig(format=FORMAT, level=logging.DEBUG)
  main(sys.argv)
