#!/usr/bin/env python2.7

from math import sin, cos, pi

if __name__ == '__main__':
  with open('sincos.s', 'w') as fh:
    print >>fh, '\tXDEF\t_sincos\n'
    print >>fh, '\tSECTION\tsincos,DATA\n'
    print >>fh, '_sincos:'

    prec = 4096

    sintab = [int(sin(i * 2 * pi / prec) * prec) for i in range(prec)]
    costab = [int(cos(i * 2 * pi / prec) * prec) for i in range(prec)]

    for a, b in zip(sintab, costab):
      print >>fh, '\tdc.w\t%d,%d' % (a, b)
