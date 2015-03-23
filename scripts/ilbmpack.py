#!/usr/bin/env python -B

import argparse
import logging
import os
import zopfli

from StringIO import StringIO
from util.ilbm import ILBM
from util.lzo import LZO

lzo = LZO()


def UnRLE(bytes_in):
  bytes_in = bytearray(bytes_in)
  bytes_out = StringIO()

  while bytes_in:
    cmd = bytes_in.pop(0)

    if cmd <= 127:
      l = cmd + 1
      s = bytes_in[:l]
      bytes_in = bytes_in[l:]
      bytes_out.write(s)
    else:
      l = 257 - cmd
      s = bytes_in.pop(0)
      bytes_out.write(chr(s) * l)

  out = bytes_out.getvalue()
  bytes_out.close()

  return out


def Deinterleave(data, width, height, depth):
  out = StringIO()
  bytesPerRow = ((width + 15) & ~15) / 8

  for i in range(depth):
    s = bytesPerRow * i
    for j in range(height):
      out.write(data[s:s + bytesPerRow])
      s += bytesPerRow * depth

  return out.getvalue()


def main():
  parser = argparse.ArgumentParser(
    description='Compresses ILBM IFF file with LZO or Deflate algorithm.')
  parser.add_argument(
    '-m', '--method', type=str, choices=['none', 'lzo', 'deflate'],
    default='lzo', help='Compression method to use.')
  parser.add_argument(
    '-f', '--force', action='store_true',
    help='If the output file exists, the tool will overwrite it.')
  parser.add_argument(
    'input', metavar='INPUT', type=str,
    help='Input ILBM IFF file name.')
  parser.add_argument(
    'output', metavar='OUTPUT', type=str, nargs='?',
    help='Output ILBM IFF file name.')
  args = parser.parse_args()

  if args.output is None:
    args.output = args.input

  if not os.path.isfile(args.input):
    raise SystemExit('File "%s" does not exists!' % args.input)

  if os.path.exists(args.output) and not args.force:
    raise SystemExit(
      'File "%s" already exists (use "-f" to override).' %
      args.output)

  ilbm = ILBM()
  ilbm.ChunkBlackList.append('CRNG')

  if ilbm.load(args.input):
    bmhd = ilbm.get('BMHD')
    body = ilbm.get('BODY')

    size = ((bmhd.data.w + 15) & ~15) / 8 * bmhd.data.h * bmhd.data.nPlanes
    payload = body.data.read()

    logging.info(bmhd.data)
    logging.info('BODY size before compression: %d/%d' % (len(payload), size))

    if bmhd.data.compression in [0, 1, 254, 255]:
      if bmhd.data.compression == 1:
        payload = UnRLE(payload)
      if bmhd.data.compression == 254:
        payload = zopfli.decompress(payload, size)
      if bmhd.data.compression == 255:
        payload = lzo.decompress(payload, size)
      compression = 0
      if args.method == 'deflate':
        opts = zopfli.Options()
        payload = zopfli.compress(opts, payload, len(payload))
        compression = 254
      if args.method == 'lzo':
        payload = lzo.compress(payload)
        compression = 255
      if args.method == 'none':
        compression = 0
      body.data = payload
      bmhd.data = bmhd.data._replace(compression=compression)
      logging.info(
        'BODY size after compression: %d/%d' % (len(body.data), size))
      ilbm.save(args.output)
    else:
      logging.warning('Unknown compression: %d' % bmhd.data.compression)


if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

  main()
