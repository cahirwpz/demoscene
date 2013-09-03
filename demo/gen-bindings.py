#!/usr/bin/env python2.7

import argparse
import logging
import re

if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  parser = argparse.ArgumentParser()
  parser.add_argument("source", help="C source file to be processed")
  args = parser.parse_args()

  callbacks = []
  parameters = []

  with open(args.source) as lines:
    for line in lines:
      if ("CALLBACK" in line) or ("PARAMETER" in line):
        words = [word.strip()
                 for word in re.split("(\s*[(){,]\s*)", line)
                 if "," not in word]
        lparen = words.index("(")
        rparen = words.index(")")
        macro = words[lparen - 1]
        args = words[lparen + 1: rparen]
        if macro == "CALLBACK":
          callbacks.append(args[0])
        if macro == "PARAMETER":
          parameters.append(args[0:2])

  print "SymbolT CallbackSymbols[] = {"
  for callback in callbacks:
    print "  {\"%s\", (void *)&%s}," % (callback, callback)
  print "  {NULL, NULL}"
  print "};"

  print "SymbolT ParameterSymbols[] = {"
  for p_type, p_name in parameters:
    print "  {\"%s\", (void *)&%s}," % (p_name, p_name)
  print "  {NULL, NULL}"
  print "};"
