#!/usr/bin/env python2.7

import argparse
import os.path
import logging

from PIL import Image
from util.quant import Color, Box, KDNode, SplitKDTree, FloydSteinberg


def QuantizeImage(image, kdtree, dithering, is_transparent):
  output = Image.new('P', image.size)

  quantized = output.load()
  width, height = image.size
  errors = 0.0

  if is_transparent:
    alpha = image.split()[-1].load()
    image = image.convert('RGB')

  pixels = image.load()

  for y in range(height):
    for x in range(width):
      if is_transparent and alpha[x, y] < 128:
        quantized[x, y] = 0
      else:
        node, diff, error = kdtree.Search(Color(*pixels[x, y]))
        errors += error
        quantized[x, y] = node.number

        if dithering:
          FloydSteinberg(pixels, x, y, width, height, diff)

  logging.info('Quantization error: %.3f.', errors / (width * height))

  return output


def Quantize(sources, colors=256):
  assert all(source.mode in ['RGB', 'RGBA'] for source in sources)

  logging.info(
    'Quantizing colorspace using median-cut algorithm to %d colors.',
    args.colors)

  pixels = []

  for source in sources:
    if source.mode is 'RGBA':
      # Exclude pixels which are deemed to be transparent.
      pixels.extend(
        [Color(r, g, b) for r, g, b, a in source.getdata() if a >= 128])
    else:
      pixels.extend([Color(r, g, b) for r, g, b in source.getdata()])

  # Remember to reserve one color for transparency.
  is_transparent = False

  if any(source.mode is 'RGBA' for source in sources):
    is_transparent = True
    colors -= 1

  space = Box(pixels, 0, len(pixels))
  kdtree = KDNode(space)
  leaves = SplitKDTree(kdtree, colors)

  if is_transparent:
    palette = [0, 0, 0]
  else:
    palette = []

  for number, leaf in enumerate(leaves):
    leaf.number = number

    # Reserve color #0 for transparency.
    if is_transparent:
      leaf.number += 1

    color = leaf.box.color

    palette.append(color.r)
    palette.append(color.g)
    palette.append(color.b)

  return (kdtree, palette)


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  parser = argparse.ArgumentParser(
    description='Converts input image to use only given number of colors.')
  parser.add_argument(
    '-c', '--colors', metavar='COLORS', type=int, default=256,
    help='Number of colors to use in output image.')
  parser.add_argument(
    '-t', '--target', metavar='DIR', type=str, default='.',
    help='Target directory where output image will be written.')
  parser.add_argument(
    '-d', '--dithering', action='store_true',
    help='Turn on Floyd-Steinberg dithering.')
  parser.add_argument(
    '-f', '--force', action='store_true',
    help='If output image exists, the tool will overwrite it.')
  parser.add_argument(
    'input', metavar='INPUT', nargs='+', help='Input image filenames.')
  args = parser.parse_args()

  images = []

  for inputPath in args.input:
    inputPath = os.path.abspath(inputPath)
    inputFilePart = os.path.splitext(os.path.basename(inputPath))[0]
    outputPath = os.path.abspath(
      os.path.join(args.target, '%s-%d.png' % (inputFilePart, args.colors)))

    if not os.path.isfile(inputPath):
      raise SystemExit('Input file does not exists!')

    logging.info('Reading input file: "%s".', inputPath)

    images.append((Image.open(inputPath), outputPath))

    if os.path.isfile(outputPath) and not args.force:
      raise SystemExit('Will not overwrite "%s" file!' % outputPath)

  kdtree, palette = Quantize([image for image, _ in images], args.colors)

  for image, outputPath in images:
    logging.info(
      'Remapping colors (%s dithering) from the original image "%s".',
      ('without', 'with')[args.dithering], image.filename)

    is_transparent = image.mode is 'RGBA'

    output = QuantizeImage(image, kdtree, args.dithering, is_transparent)
    output.putpalette(palette)

    logging.info('Saving quantized image to: "%s".', outputPath)

    attributes = {'transparency': 0} if is_transparent else {}
    output.save(outputPath, **attributes)
