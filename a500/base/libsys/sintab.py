#!/usr/bin/env python3

from math import sin, pi

if __name__ == '__main__':
  with open('sintab.c', 'w') as fh:
    prec = 4096

    print('#include "common.h"\n', file=fh)
    print('WORD sintab[%d] = {' % prec, file=fh)

    sintab = [int(sin(i * 2 * pi / prec) * prec) for i in range(prec)]
    prev = 0

    for a in sintab:
      print('\t%d,' % (a - prev), file=fh)
      prev = a

    print('};', file=fh)
    print('\nvoid InitSinTab() {', file=fh)
    print('  WORD sum = 0, n = %d;' % prec, file=fh)
    print('  WORD *tab = sintab;', file=fh)
    print('  Log("[Init] Preparing sinus table\n");', file=fh)
    print('  while (--n >= 0) {', file=fh)
    print('    sum += *tab;', file=fh)
    print('    *tab++ = sum;', file=fh)
    print('  }', file=fh)
    print('}', file=fh)
    print('\nADD2INIT(InitSinTab, 0);', file=fh)
