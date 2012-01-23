#!/usr/bin/env python

import argparse
import os
import subprocess
import sys

def main():
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

  command = ['convert', inputPath]

  if args.dithering:
    command.extend(['-dither', 'FloydSteinberg'])
  else:
    command.append('+dither')

  command.extend(['-colors', str(args.colors), outputPath])

  subprocess.call(command, stdout=sys.stdout, stderr=sys.stderr)

if __name__ == '__main__':
  main()
