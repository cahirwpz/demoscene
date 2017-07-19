#!/usr/bin/env python

import logging
import sys

from iff.ilbm import ILBM
from pprint import pprint


def main():
  ilbm = ILBM()
  ilbm.ChunkBlackList.append('JUNK')

  for path in sys.argv[1:]:
    ilbm.load(path)
    for chunk in ilbm:
      pprint((chunk.name, chunk.data))


if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
  main()
