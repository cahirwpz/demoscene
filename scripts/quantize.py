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


def Quantize(inputPath, outputPath, colors=256, dithering=False):
  logging.info('Reading input file: "%s".', inputPath)

  image = Image.open(inputPath)

  assert image.mode in ['RGB', 'RGBA']

  logging.info('Quantizing colorspace using median-cut algorithm.')

  is_transparent = image.mode is 'RGBA'

  if is_transparent:
    # Exclude pixels which are deemed to be transparent.
    pixels = [Color(r, g, b) for r, g, b, a in image.getdata() if a >= 128]
    # Remember to reserve one color for transparency.
    colors -= 1
  else:
    pixels = [Color(r, g, b) for r, g, b in image.getdata()]

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

  logging.info('Remapping colors (%s dithering) from the original image.',
               ('without', 'with')[dithering])

  output = QuantizeImage(image, kdtree, dithering, is_transparent)
  output.putpalette(palette)

  logging.info('Saving quantized image to: "%s".', outputPath)

  attributes = {'transparency': 0} if is_transparent else {}
  output.save(outputPath, **attributes)


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  parser = argparse.ArgumentParser(
    description='Converts input image to use only given number of colors.')
  parser.add_argument(
    '-c', '--colors', metavar='COLORS', type=int, default=256,
    help='Number of colors to use in output image.')
  parser.add_argument(
    '-d', '--dithering', action='store_true',
    help='Turn on Floyd-Steinberg dithering.')
  parser.add_argument(
    '-f', '--force', action='store_true',
    help='If output image exists, the tool will overwrite it.')
  parser.add_argument(
    'input', metavar='INPUT', type=str, help='Input image filename.')
  parser.add_argument(
    'output', metavar='OUTPUT', type=str, help='Output image filename.')
  args = parser.parse_args()

  inputPath = os.path.abspath(args.input)
  outputPath = os.path.abspath(args.output)

  if not os.path.isfile(inputPath):
    raise SystemExit('Input file does not exists!')

  if os.path.isfile(outputPath) and not args.force:
    raise SystemExit('Will not overwrite output file!')

  if inputPath == outputPath:
    raise SystemExit('Input and output files have to be different!')

  Quantize(inputPath, outputPath, args.colors, args.dithering)
