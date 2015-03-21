#!/usr/bin/env python

from array import array
from struct import pack
from StringIO import StringIO

import json
import argparse
import os
import shutil


def makestr(s):
  s = str(s) + '\0'
  if len(s) & 1:
    s += '\0'
  return s


def convert(obj, lwobFile):
  lwob = StringIO()
  srfs = StringIO()
  pols = array('H')
  pnts = array('f')

  for surface in obj['surf']:
    srfs.write(makestr(surface['name']))

  srfs = srfs.getvalue()
  lwob.write(pack('!4sI', 'SRFS', len(srfs)))
  lwob.write(srfs)

  for x, y, z in obj['pnts']:
    pnts.append(x)
    pnts.append(y)
    pnts.append(z)

  pnts.byteswap()
  lwob.write(pack('!4sI', 'PNTS', len(pnts) * 4))
  lwob.write(pnts.tostring())

  for indices, surface in obj['pols']:
    pols.append(len(indices))
    pols.extend(indices)
    pols.append(surface)

  pols.byteswap()
  lwob.write(pack('!4sI', 'POLS', len(pols) * 2))
  lwob.write(pols.tostring())

  for surface in obj['surf']:
    surf = StringIO()
    surf.write(makestr(surface['name']))
    surf.write(pack('!4sHBBBx', 'COLR', 4, *surface['color']))
    surf = surf.getvalue()
    lwob.write(pack('!4sI', 'SURF', len(surf)))
    lwob.write(surf)

  chunks = lwob.getvalue()
  lwobFile.write(pack('!4sI4s', 'FORM', len(chunks), 'LWOB'))
  lwobFile.write(chunks)

if __name__ == '__main__':
  parser = argparse.ArgumentParser(
      description='Fixes PNG accordingly with passed options.')
  parser.add_argument('json', metavar='JSON', type=str,
                      help='Input file.')
  parser.add_argument('lwob', metavar='LWOB', type=str,
                      help='Output file.')
  args = parser.parse_args()

  if not os.path.isfile(args.json):
    raise SystemExit('File "%s" does not exists!' % args.json)

  if os.path.isfile(args.lwob):
    backup = args.lwob + '.bak'

    if os.path.exists(backup):
      raise SystemExit('Backup file "%s" exists.' % backup)

    shutil.copy(args.lwob, backup)

  with open(args.json, 'r') as jsonFile:
    with open(args.lwob, 'wb') as lwobFile:
      convert(json.load(jsonFile), lwobFile)
