#!/usr/bin/env python3
from contextlib import redirect_stdout

if __name__ == '__main__':
  with open('colortab.s', 'w') as fh:
    with redirect_stdout(fh):
      print('\tXDEF\t_colortab\n')
      print('\tSECTION\tcolortab,DATA\n')
      print('_colortab:')

      for i in range(16):
        for j in range(16):
          row = ', '.join(
              ["${:02x}".format((i + (j - i) * k // 15) * 16)
               for k in range(16)])
          print('\tdc.b\t' + row)
