#!/usr/bin/env python3

import os.path
import sys
from collections import defaultdict

if __name__ == '__main__':
    if len(sys.argv) < 2:
        raise SystemExit(f'Usage: {sys.argv[0]} source.txt')
    txtFile = sys.argv[1]
    chunks = defaultdict()
    chunkName = None

    with open(txtFile) as f:
        for line in f:
            if line.startswith(';>>> '):
                chunkName = line.rstrip()[5:]
            elif line.startswith(';'):
                continue
            else:
                chunks[chunkName] = chunks.get(chunkName, '') + line
    for key in ['startrow', 'endrow', 'plot-even', 'plot-odd', 'add-dst',
                'addq-dst', 'map']:
        if key not in chunks:
            raise SystemExit(f'Chunk "{key}" not found in {txtFile}')

    dst_inc = 0
    crossedrow = False
    print('tx_rotzoom:')
    for line in chunks['map'].splitlines():
        print(chunks['startrow'])
        line = line.strip()
        even = True
        for ch in line:
            if ch == '#':
                if dst_inc > 0:
                    if dst_inc <= 8 and not crossedrow:
                        dst_inc_chunk = 'addq-dst'
                    else:
                        dst_inc_chunk = 'add-dst'
                    if crossedrow:
                        dst_value = f'{dst_inc}+SINGLEROW_SIZE'
                    else:
                        dst_value = str(dst_inc)
                    print(chunks[dst_inc_chunk].replace('$BYTES', dst_value))
                    crossedrow = False
                    dst_inc = 0
                if even:
                    print(chunks['plot-even'])
                else:
                    print(chunks['plot-odd'])
                    dst_inc += 4
            elif ch == '.':
                print(chunks['plot-skip'])
                if not even:
                    dst_inc += 4
            elif ch == ':':
                if not even:
                    dst_inc += 4
            elif ch == ' ':
                break
            else:
                raise SystemExit(f'Unexpected char "{ch}" in "{line}"')
            even = not even
        dst_inc += 16
        crossedrow = True
        print(chunks['endrow'])
    print('\trts')
    print('tx_copy:')
    row_num = 0
    copper_pos = 6 # skip: yy01 fffe 0186
    for line in chunks['map'].splitlines():
        row_pos = copper_pos
        print('; %02d [%s]' % (row_num, line))
        even = True
        for ch in line:
            if ch == '#':
                row_add = '+SINGLEROW_SIZE' if even else ''
                print('\tmove.w\t%d%s(a0),%d%s(a1)' % (row_pos, row_add, row_pos, row_add))
            elif ch == ' ':
                break
            elif ch != '.' and ch != ':':
                raise SystemExit(f'Unexpected char "{ch}" in "{line}"')
            if not even:
                row_pos += 4
            even = not even
        row_num += 1
        copper_pos += 16*4*2
    print('\trts')
