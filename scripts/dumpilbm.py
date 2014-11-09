#!/usr/bin/env python

import logging
import sys

from util.ilbm import ILBM
from pprint import pprint


def main():
  ilbm = ILBM()

  for path in sys.argv[1:]:
    ilbm.load(path)
    for chunk in ilbm:
      pprint(chunk)


if __name__ == '__main__':
  logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
  main()
