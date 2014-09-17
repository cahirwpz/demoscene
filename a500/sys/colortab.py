#!/usr/bin/env python -B

if __name__ == '__main__':
  with open('colortab.s', 'w') as fh:
    print >>fh, '\tXDEF\t_colortab\n'
    print >>fh, '\tSECTION\tcolortab,DATA\n'
    print >>fh, '_colortab:'

    for i in range(16):
      for j in range(16):
        row = ', '.join([str(i + (j - i) * k / 15) for k in range(16)])
        print >>fh, '\tdc.b\t' + row
