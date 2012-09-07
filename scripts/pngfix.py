#!/usr/bin/env python

import Image
import argparse
import os
import shutil
import sys

def main():
  parser = argparse.ArgumentParser(
      description='Fixes PNG accordingly with passed options.')
  parser.add_argument('--transparency-fix', action='store_true',
      help='Force color #0 to be transparency color.')
  parser.add_argument('--drop-alpha', action='store_true',
      help='Remove alpha channel from RGBA image.')
  parser.add_argument('files', metavar='FILES', type=str, nargs='+',
      help='PNG image filenames.')
  args = parser.parse_args()

  for path in args.files:
    if not os.path.isfile(path):
      raise SystemExit('File "%s" does not exists!' % path)

    backup =  path + '.bak'

    if os.path.exists(backup):
      raise SystemExit('Backup file "%s" exists.' % backup)

    shutil.copy(path, backup)

    im = Image.open(path)

    if im.mode == 'P' and args.transparency_fix:
      im.save(path, transparency=0)

    if im.mode == 'RGBA' and args.drop_alpha:
      im = im.convert('RGB')
      im.save(path)

if __name__ == '__main__':
  main()
