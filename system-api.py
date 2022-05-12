#!/usr/bin/env python3

import argparse


def doit(api, lds):
    syscalls = []
    shvars = []

    for i, line in enumerate(map(str.strip, api)):
        if not line:
            continue
        if line.startswith(';'):
            continue
        fs = line.split()
        if fs[0] == 'syscall' and len(fs) == 2:
            syscalls.append(fs[1])
        elif fs[0] == 'shvar' and len(fs) == 3:
            shvars.append([fs[1], int(fs[2])])
        else:
            raise SystemExit(f'line {i+1}: syntax error: {line}')

    for i, sc in enumerate(syscalls):
        addr = i * 4 + 0xc0
        print(f'PROVIDE(__{sc} = 0x{addr:x});', file=lds)

    addr = 0x400
    for var, sz in sorted(shvars, key=lambda x: -x[1]):
        addr -= sz
        print(f'PROVIDE(_{var} = 0x{addr:x});', file=lds)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate linker script for system API.')
    parser.add_argument(
        'api', metavar='API', type=str, help='API definition file')
    parser.add_argument(
        'lds', metavar='LDS', type=str, help='GNU linker script file')
    args = parser.parse_args()

    with open(args.api) as api:
        with open(args.lds, 'w') as lds:
            doit(api, lds)
