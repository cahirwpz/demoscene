#!/usr/bin/env python2.7

from operator import attrgetter

import Image
import logging
import argparse
import os

class MyImage(object):
  def __init__(self, path):
    self.name = os.path.basename(path)
    self.absPath = os.path.abspath(path)
    self.startColor = 0

    try:
      logging.info('Loading image "%s".', self.name)
      self.im = Image.open(self.absPath)
    except IOError as ex:
      raise SystemExit('Error: %s.' % ex)

    logging.info('Image "%s" has %d colors.', self.name, self.uniqueColors)

  @property
  def uniqueColors(self):
    return len(self.im.getcolors())

  @property
  def usedColors(self):
    return sorted([num for _, num in self.im.getcolors()])

  def getColor(self, num):
    return self.im.getpalette()[num*3:(num+1)*3]

  def remap(self, palette):
    self.im.putpalette(palette)

    colorMap = dict(
        (y,x) for x,y in enumerate(self.usedColors, start=self.startColor))

    self.im.putdata([colorMap[p] for p in self.im.getdata()])

  def save(self, filename):
    self.im.save(filename)
    logging.info('Remapped image "%s" saved to "%s" file.', self.name, filename)


def main():
  parser = argparse.ArgumentParser(
      description=('Merges palettes from several images and remaps pixels to'
        ' match colors.'))
  parser.add_argument('-f', '--force', action='store_true',
      help='If output image exists, the tool will overwrite it.')
  parser.add_argument('-o', '--output', type=str, default='%',
      help=('Pattern thet is used to give name to output files. '
        'Use "%%" character to mark place where to insert original file name '
        '(without extension). Original file extension will be preserved. '
        '(Example: "my-%%-new" for "image.png" will give '
        '"my-image-new.png")'))
  parser.add_argument('-t', '--target', type=str, default='.',
      help='Directory where output files will be saved.')
  parser.add_argument('input', metavar='IMAGE', type=str, nargs='+',
      help='Input image filename.')
  args = parser.parse_args()

  if args.output.count('%') != 1:
    raise SystemExit('File name pattern "%s" must contain exactly one "%%" '
        'character.' % args.output)

  for path in args.input:
    if not os.path.isfile(path):
      raise SystemExit('Input file "%s" does not exists!' % path)

  images = map(MyImage, args.input)

  if sum(map(attrgetter('uniqueColors'), images)) > 256:
    raise SystemExit('Cannot create merged palette if images have summarized'
                     ' number of colors more than 256.')

  logging.info('Calculating merged palette.')

  commonPalette = []
  unusedColor = 0

  for image in images:
    image.startColor = unusedColor

    for color in image.usedColors:
      commonPalette.extend(image.getColor(color))

    unusedColor += len(image.usedColors)

  for i in range(unusedColor, 256):
    commonPalette.extend((i,i,i))

  logging.info('Remapping images.')

  prefix, suffix = args.output.split('%')

  for image in images:
    root, ext = os.path.splitext(image.name)
    filename = "".join([prefix, root, suffix, ext])
    path = os.path.join(args.target, filename)

    image.remap(commonPalette)

    if not os.path.exists(args.target):
      os.makedirs(args.target)

    if not os.path.exists(path) or args.force:
      image.save(path)
    else:
      logging.warning('Skipped saving "%s" image - file already exists (use'
          ' "-f" to override).', path)


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  main()
