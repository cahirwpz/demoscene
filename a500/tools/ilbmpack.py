#!/usr/bin/env python3

import argparse
import logging
import os
from zopfli.zlib import compress
from zlib import decompress

from iff.ilbm import ILBM, UnRLE


def main():
  parser = argparse.ArgumentParser(
    description='Compresses ILBM IFF file with Deflate algorithm.')
  parser.add_argument(
    '-m', '--method', type=str, choices=['none', 'deflate'],
    default='deflate', help='Compression method to use.')
  parser.add_argument(
    '-q', '--quiet', action='store_true',
    help='Silence out diagnostic messages.')
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

  logLevel = [logging.INFO, logging.WARNING][args.quiet]
  logging.basicConfig(level=logLevel, format="%(levelname)s: %(message)s")

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
        payload = decompress(payload)
      compression = 0
      if args.method == 'deflate':
        # Default ZopfliZlibCompress compression used, should be ZopfliDeflate.
        # Py-zopfli do not provide api for ZopfliDeflate compression.
        payload = compress(payload)
        compression = 254
      if args.method == 'none':
        compression = 0
          # Removed zlib container bytes.
      body.data = payload[2:len(payload)-4]
      bmhd.data = bmhd.data._replace(compression=compression)
      logging.info(
        'BODY size after compression: %d/%d' % (len(body.data), size))
      ilbm.save(args.output)
    else:
      logging.warning('Unknown compression: %d' % bmhd.data.compression)


if __name__ == '__main__':

  main()
