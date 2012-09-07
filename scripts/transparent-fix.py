#!/usr/bin/env python

import Image
import os
import shutil
import sys

def main():
  for path in sys.argv[1:]:
    backup =  path + '.bak'

    if not os.path.exists(backup):
      shutil.copy(path, backup)
      im = Image.open(path)
      im.save(path, transparency=0)
    else:
      raise SystemExit('Backup file "%s" exists.' % backup)

if __name__ == '__main__':
  main()
