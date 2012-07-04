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

  def Clamp(self):
    r, g, b = self.r, self.g, self.b

    if r < 0:   r = 0
    if r > 255: r = 255
    if g < 0:   g = 0
    if g > 255: g = 255
    if b < 0:   b = 0
    if b > 255: b = 255

    return Color(r, g, b)

  def __add__(self, other):
    return Color(self.r + other.r, self.g + other.g, self.b + other.b)

  def __sub__(self, other):
    return Color(self.r - other.r, self.g - other.g, self.b - other.b)

  def __mul__(self, s):
    return Color(self.r * s, self.g * s, self.b * s) 

  def __div__(self, s):
    hs = s >> 1
    return Color((self.r + hs) / s, (self.g + hs) / s, (self.b + hs) / s)

  def __len__(self):
    return self.r * self.r + self.g * self.g + self.b * self.b


class Box(object):
  __slots__ = ('data', 'begin', 'end', 'count', 'average', 'weight', 'color', 'axis')

  def __init__(self, data, begin, end, average=None):
    assert begin < end

    self.data = data
    self.begin = begin
    self.end = end
    self.count = self.end - self.begin
    self.average = average or self.CalcAverage()

    variance = self.CalcVariance()

    self.weight = max(variance)
    self.axis = variance.index(self.weight)

  @property
  def color(self):
    return self.average / self.count

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
      d = pixel - avg
      dr, dg, db = d.r * d.r, d.g * d.g, d.b * d.b
      r += dr
      g += dg
      b += db

    return Color(r, g, b)

  def Split(self):
    axis = self.axis
    median = self.color[axis]
    data = self.data

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

    boxL = Box(self.data, self.begin, i + 1)
    boxR = Box(self.data, i + 1, self.end, average=(self.average - boxL.average))

    return (axis, median, boxL, boxR)


class KDNode(object):
  __slots__ = ('axis', 'median', 'box', 'left', 'right', 'number')

  def __init__(self, box):
    self.box = box
    self.number = -1

  def __cmp__(self, other):
    return cmp(other.box.weight, self.box.weight)

  @property
  def isLeaf(self):
    return self.box != None

  def Split(self):
    self.axis, self.median, left, right = self.box.Split()
    self.left = KDNode(left)
    self.right = KDNode(right)
    self.box = None

  def Search(self, color):
    def Recurse(node):
      if node.isLeaf:
        diff = color - node.box.color
        return node.number, diff, sqrt(len(diff))

      side = int(color[node.axis] >= node.median)
      child = (node.left, node.right)[side]
      n, diff, error = Recurse(child)

      if error > abs(color[node.axis] - node.median):
        if child is node.left:
          alternative = node.right
        else:
          alternative = node.left

        alt_n, alt_diff, alt_error = Recurse(alternative)

        if alt_error < error:
          n = alt_n
          diff = alt_diff
          error = alt_error

      return n, diff, error

    return Recurse(self)


def FloydSteinberg(pixels, pos, size, diff):
  x, y = pos
  width, height = size

  if x < width - 1:
    pixels[x+1, y] = (Color(*pixels[x+1, y]) + diff * 7 / 16).Clamp()

  if y < height - 1:
    if x > 0:
      pixels[x-1, y+1] = (Color(*pixels[x-1, y+1]) + diff * 3 / 16).Clamp()

    pixels[x, y+1] = (Color(*pixels[x, y+1]) + diff * 5 / 16).Clamp()

    if x < width - 1:
      pixels[x+1, y+1] = (Color(*pixels[x+1, y+1]) + diff / 16).Clamp()


def QuantizeImage(image, kdtree, dithering):
  output = Image.new('P', image.size)

  pixels = image.load()
  quantized = output.load()
  width, height = image.size
  errors = 0.0

  for y in range(height):
    for x in range(width):
      n, diff, error = kdtree.Search(Color(*pixels[x, y]))
      errors += error
      quantized[x, y] = n

      if dithering:
        FloydSteinberg(pixels, (x, y), image.size, diff)

  logging.info('Quantization error: %.3f.', errors / (width * height))

  return output


def Quantize(inputPath, outputPath, colors=256, dithering=False):
  logging.info('Reading input file: "%s".', inputPath)

  image = Image.open(inputPath)

  logging.info('Splitting colorspace using median-cut algorithm.')

  data = [Color(r, g, b) for r, g, b in image.getdata()]
  kdtree = KDNode(Box(data, 0, len(data)))
  leaves = [kdtree]

  while len(leaves) < colors:
    leaf = heappop(leaves)
    leaf.Split()
    heappush(leaves, leaf.left)
    heappush(leaves, leaf.right)

  leaves = sorted(leaves, key=lambda l: sum(l.box.color))

  for number, leaf in enumerate(leaves):
    leaf.number = number

  candidates = [leaf.box.color for leaf in leaves]

  logging.info('Quantizing colors (%s dithering) in the original image.',
               ('without', 'with')[dithering])

  output = QuantizeImage(image, kdtree, dithering)
  output.putpalette(reduce(lambda x, y: x + y, map(list, candidates)))

  logging.info('Saving quantized image to: "%s".', outputPath)

  output.save(outputPath)


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
