#!/usr/bin/env python -B

from math import sin, pi

if __name__ == '__main__':
  with open('sintab.c', 'w') as fh:
    prec = 4096

    print >>fh, '#include "common.h"\n'
    print >>fh, 'WORD sintab[%d] = {' % prec

    sintab = [int(sin(i * 2 * pi / prec) * prec) for i in range(prec)]
    prev = 0

    for a in sintab:
      print >>fh, '\t%d,' % (a - prev)
      prev = a

    print >>fh, '};'
    print >>fh, '\nvoid InitSinTab() {'
    print >>fh, '  WORD sum = 0, n = %d;' % prec
    print >>fh, '  WORD *tab = sintab;'
    print >>fh, '  Log("[Init] Preparing sinus table\n");'
    print >>fh, '  while (--n >= 0) {'
    print >>fh, '    sum += *tab;'
    print >>fh, '    *tab++ = sum;'
    print >>fh, '  }'
    print >>fh, '}'
    print >>fh, '\nADD2INIT(InitSinTab, 0);'
