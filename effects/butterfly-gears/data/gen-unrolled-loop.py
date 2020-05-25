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
        dst_inc += 8
        crossedrow = True
        print(chunks['endrow'])
