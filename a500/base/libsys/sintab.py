#!/usr/bin/env python -B

from math import sin, pi

if __name__ == '__main__':
  with open('sintab.s', 'w') as fh:
    print >>fh, '\tXDEF\t_sintab\n'
    print >>fh, '\tSECTION\tsintab,DATA\n'
    print >>fh, '_sintab:'

    prec = 4096

    sintab = [int(sin(i * 2 * pi / prec) * prec) for i in range(prec)]

    for a in sintab:
      print >>fh, '\tdc.w\t%d' % a
