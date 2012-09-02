#!/usr/bin/env python

from array import array

import wave
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
    sound = wave.open(inputPath)
  except IOError as ex:
    raise SystemExit('Error: %s.' % ex)

  bit = sound.getsampwidth()
  stereo = (sound.getnchannels() == 2)
  frameRate = sound.getframerate()
  frames = sound.getnframes()

  info = ["%.1f s" % (float(frames) / frameRate),
          "stereo" if stereo else "mono",
          "%d bit" % (bit * 8),
          "%d Hz" % frameRate]

  print "%s:" % inputPath, ", ".join(info)

  rawSoundFile = outputPath + '.snd'

  if os.path.isfile(rawSoundFile) and not args.force:
    raise SystemExit('Will not overwrite output file!')

  with open(rawSoundFile, 'w') as soundFile:
    fmt = 'h' if bit == 2 else 'b'

    samples = array(fmt)

    if stereo:
      fmt = fmt * 2

    samples.fromstring(sound.readframes(frames))
    samples.byteswap()

    flags = 0
    if stereo:
      flags |= 1 
    if bit == 2:
      flags |= 2

    soundFile.write(struct.pack('>HHI', flags, frameRate, frames))
    soundFile.write(samples.tostring())

if __name__ == '__main__':
  main()
