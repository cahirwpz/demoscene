#!/usr/bin/env python3

from math import sin, pi
from contextlib import redirect_stdout

if __name__ == '__main__':
    with open('sintab.c', 'w') as fh:
        with redirect_stdout(fh):
            prec = 4096

            print('#include "common.h"\n')
            print('#include "debug.h"\n')
            print('short sintab[%d] = {' % prec)

            sintab = [int(sin(i * 2 * pi / prec) * prec) for i in range(prec)]
            prev = 0

            for a in sintab:
                print('\t%d,' % (a - prev))
                prev = a

            print('};')
            print('\nvoid InitSinTab(void) {')
            print('  short sum = 0, n = %d;' % prec)
            print('  short *tab = sintab;')
            print('  Log("[Init] Preparing sinus table\\n");')
            print('  while (--n >= 0) {')
            print('    sum += *tab;')
            print('    *tab++ = sum;')
            print('  }')
            print('}')
            print('\nADD2INIT(InitSinTab, 0);')
