#!/usr/bin/env python3

import logging
import sys

from uaedbg.hunk import ReadHunkFile


if __name__ == '__main__':
    logging.basicConfig()

    for path in sys.argv[1:]:
        print('Parsing "%s".' % path)
        print('')

        for h in ReadHunkFile(path):
            h.dump()
            print('')
