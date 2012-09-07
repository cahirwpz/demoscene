#!/usr/bin/env python2.7

import argparse
import os.path
import logging

from collections import namedtuple
from heapq import heappop, heappush
from math import sqrt
from PIL import Image


class Color(namedtuple('Color', 'r g b')):
  __slots__ = ()

  def __sub__(self, other):
    return Color(self.r - other.r, self.g - other.g, self.b - other.b)

  def __div__(self, s):
    s = float(s)
    return Color(int(self.r / s), int(self.g / s), int(self.b / s))


class Box(object):
  __slots__ = ('data', 'begin', 'end', 'count', 'average', 'weight', 'color', 'axis')

  def __init__(self, data, begin, end, average=None):
    assert begin < end

    self.data = data
    self.begin = begin
    self.end = end
    self.average = average or self.CalcAverage()
    self.count = self.end - self.begin
    self.color = self.average / self.count

    variance = self.CalcVariance()

    self.weight = max(variance)
    self.axis = variance.index(self.weight)

  def __repr__(self):
    return '[%d..%d], count: %d, weight: %s' % (self.begin, self.end, self.count, self.weight)

  def CalcAverage(self):
    r, g, b = 0, 0, 0

    for pixel in self.data[self.begin:self.end]:
      r += pixel.r
      g += pixel.g
      b += pixel.b

    return Color(r, g, b)

  def CalcVariance(self):
    r, g, b = 0, 0, 0
    avg = self.color

    for pixel in self.data[self.begin:self.end]:
      dr, dg, db = pixel.r - avg.r, pixel.g - avg.g, pixel.b - avg.b
      r += dr * dr
      g += dg * dg
      b += db * db

    return Color(r, g, b)

  def Split(self):
    axis = self.axis
    median = self.color[axis]
    data = self.data

    # Sometimes average color is not a good median (ie. when a single value
    # dominates).  In such cases we need to take the other value.
    values = sorted(set(item[axis] for item in data[self.begin:self.end]))

    if median == values[-1]:
      median = values[-2]
    if median == values[0]:
      median = values[1]

    i = self.begin
    j = self.end - 1

    while i < j:
      while data[i][axis] < median and i < j:
        i += 1
      while data[j][axis] >= median and i < j:
        j -= 1

      tmp = data[i]
      data[i] = data[j]
      data[j] = tmp

    boxL = Box(self.data, self.begin, i)

    averageR = self.average - boxL.average
    boxR = Box(self.data, i, self.end)

    return (axis, median, boxL, boxR)


class KDNode(object):
  __slots__ = ('axis', 'median', 'box', 'left', 'right', 'number')

  def __init__(self, box):
    self.box = box
    self.number = -1

  def __cmp__(self, other):
    return cmp(other.box.weight, self.box.weight)

  def __repr__(self):
    return repr(self.box)

  def Split(self):
    self.axis, self.median, left, right = self.box.Split()
    self.left = KDNode(left)
    self.right = KDNode(right)
    self.box = None

  def Search(self, color):
    if self.box:
      avg = self.box.color
      r, g, b = color[0] - avg.r, color[1] - avg.g, color[2] - avg.b
      return self, Color(r, g, b), sqrt(r*r + g*g + b*b)

    dist = color[self.axis] - self.median

    if dist < 0:
      child, alt_child = self.left, self.right
    else:
      child, alt_child = self.right, self.left

    node, diff, error = child.Search(color)

    if error > abs(dist):
      alt_node, alt_diff, alt_error = alt_child.Search(color)

      if alt_error < error:
        node, diff, error = alt_node, alt_diff, alt_error

    return node, diff, error


def LumaCCIR601(color):
  return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b


def SplitKDTree(kdtree, leavesNum):
  leaves = [kdtree]

  while len(leaves) < leavesNum:
    leaf = heappop(leaves)
    leaf.Split()
    heappush(leaves, leaf.left)
    heappush(leaves, leaf.right)

  return sorted(leaves, key=lambda l: LumaCCIR601(l.box.color))


def AddErrorAndClamp(pixel, error, coeff):
  r, g, b = pixel

  r += (error.r * coeff + 7) / 16
  g += (error.g * coeff + 7) / 16
  b += (error.b * coeff + 7) / 16

  if r < 0:   r = 0
  if r > 255: r = 255
  if g < 0:   g = 0
  if g > 255: g = 255
  if b < 0:   b = 0
  if b > 255: b = 255

  return r, g, b


def FloydSteinberg(pixels, pos, size, error):
  x, y = pos
  width, height = size

  if x < width - 1:
    pixels[x+1, y] = AddErrorAndClamp(pixels[x+1, y], error, 7)

  if y < height - 1:
    if x > 0:
      pixels[x-1, y+1] = AddErrorAndClamp(pixels[x-1, y+1], error, 3)

    pixels[x, y+1] = AddErrorAndClamp(pixels[x, y+1], error, 5)

    if x < width - 1:
      pixels[x+1, y+1] = AddErrorAndClamp(pixels[x+1, y+1], error, 1)


def QuantizeImage(image, kdtree, dithering, is_transparent):
  output = Image.new('P', image.size)

  pixels = image.load()
  quantized = output.load()
  width, height = image.size
  errors = 0.0

  if is_transparent:
    alpha = image.split()[-1].load()
    image = image.convert('RGB')

  for y in range(height):
    for x in range(width):
      if is_transparent and alpha[x, y] < 128:
        quantized[x, y] = 0
      else:
        node, diff, error = kdtree.Search(pixels[x, y])
        errors += error
        quantized[x, y] = node.number

        if dithering:
          FloydSteinberg(pixels, (x, y), image.size, diff)

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

    palette.extend(leaf.box.color)

  logging.info('Remapping colors (%s dithering) from the original image.',
               ('without', 'with')[dithering])

  output = QuantizeImage(image, kdtree, dithering, is_transparent)
  output.putpalette(palette)

  logging.info('Saving quantized image to: "%s".', outputPath)

  attributes = {'transparency': 0} if is_transparent else {}
  output.save(outputPath, **attributes)


def Main():
  parser = argparse.ArgumentParser(
      description='Converts input image to use only given number of colors.')
  parser.add_argument('-c', '--colors', metavar='COLORS', type=int,
      default=256, help='Number of colors to use in output image.')
  parser.add_argument('-d', '--dithering', action='store_true',
      help='Turn on Floyd-Steinberg dithering.')
  parser.add_argument('-f', '--force', action='store_true',
      help='If output image exists, the tool will overwrite it.')
  parser.add_argument('input', metavar='INPUT', type=str,
      help='Input image filename.')
  parser.add_argument('output', metavar='OUTPUT', type=str,
      help='Output image filename.')
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


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  Main()
