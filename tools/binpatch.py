#!/usr/bin/env python3

import argparse
import logging


def patch(bin, diff):
  pos = 0
  for cmd in diff:
    if cmd[0] == '@':
      pos = int(cmd[1], 16)
      logging.debug('move to 0x%x' % pos)
    elif cmd[0] == '-':
      data = bytearray.fromhex(cmd[1])
      logging.debug('remove %s' % cmd[1])
      size = len(data)
      if bin[pos: pos + size] != data:
        logging.error('no match at $%x' % pos)
        raise SystemExit
      del bin[pos: pos + size]
    elif cmd[0] == '+':
      data = bytearray.fromhex(cmd[1])
      logging.debug('insert %s' % cmd[1])
      size = len(data)
      bin[pos:pos] = data
      pos += size
  return bin


if __name__ == '__main__':
  logging.basicConfig(level=logging.WARN, format='%(levelname)s: %(message)s')

  parser = argparse.ArgumentParser(
      description='Converts an image to ILBM format.')
  parser.add_argument('input', metavar='INPUT', type=str,
                      help='Input binary filename.')
  parser.add_argument('diff', metavar='DIFF', type=str,
                      help='Patch filename.')
  parser.add_argument('output', metavar='OUTPUT', type=str,
                      help='Output binary filename.')
  args = parser.parse_args()

  with open(args.input, 'rb') as input:
    bin = bytearray(input.read())
    logging.info('Input size %ld', len(bin))

    with open(args.diff, 'r') as diff:
      diff = map(lambda x: x.strip().split(), diff.readlines())

      with open(args.output, 'wb') as output:
        bout = patch(bin, diff)
        logging.info('Output size %ld', len(bout))
        output.write(bout)
