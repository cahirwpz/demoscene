#!/usr/bin/env python2.7

import argparse
import logging
import re

from collections import namedtuple

Symbol = namedtuple('Symbol', 'name addr')


if __name__ == '__main__':
  logging.basicConfig(level=logging.DEBUG, format="%(levelname)s: %(message)s")

  parser = argparse.ArgumentParser()
  parser.add_argument("source", help="C source file to be processed")
  args = parser.parse_args()

  callbacks = []
  parameters = []
  arrays = []

  with open(args.source) as lines:
    for line in lines:
      if any(pat in line for pat in ["CALLBACK", "PARAMETER", "ARRAY"]):
        words = [word.strip()
                 for word in re.split("(\s*[(){,]\s*)", line)
                 if "," not in word]
        lparen = words.index("(")
        rparen = words.index(")")
        macro = words[lparen - 1]
        args = words[lparen + 1: rparen]
        if macro == "CALLBACK":
          callbacks.append(Symbol(args[0], args[0]))
        if macro == "PARAMETER":
          parameters.append(Symbol(args[1], args[1]))
        if macro == "ARRAY":
          arrays.append(Symbol(args[2], args[2]))

  print "SymbolT CallbackSymbols[] = {"
  for callback in callbacks:
    print "  {\"%s\", (void *)&%s}," % (callback.name, callback.addr)
  print "  {NULL, NULL}"
  print "};"

  print "SymbolT ParameterSymbols[] = {"
  for parameter in parameters:
    print "  {\"%s\", (void *)&%s}," % (parameter.name, parameter.addr)
  for array in arrays:
    print "  {\"%s\", (void *)%s}," % (array.name, array.addr)
  print "  {NULL, NULL}"
  print "};"
