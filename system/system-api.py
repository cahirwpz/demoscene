#!/usr/bin/env python3

import argparse
from contextlib import redirect_stdout
from io import StringIO


def doit(api, sc, jv):
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

    with redirect_stdout(sc):
        for i, sc in enumerate(syscalls):
            addr = i * 4 + 0xc0
            print(f'PROVIDE(__{sc} = 0x{addr:x});')

        addr = 0x400
        for var, sz in sorted(shvars, key=lambda x: -x[1]):
            addr -= sz
            print(f'PROVIDE(_{var} = 0x{addr:x});')

    with redirect_stdout(jv):
        print("#include <asm.h>")
        print()
        print(f"ENTRY(JumpTable)")
        for sc in syscalls:
            print(f"\tjmp\t_L({sc})")
        print(f"END(JumpTable)")
        print()
        print("\t.set\t_L(JumpTableSize), . - _L(JumpTable)")
        print()
        print("# vim: ft=gas:ts=8:sw=8:noet")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Generate linker script for system API.')
    parser.add_argument(
        'api', metavar='API', type=str,
        help='API definition file with system calls and shared variables')
    parser.add_argument(
        'sc_lds', metavar='SCLDS', type=str,
        help='GNU linker script file with system calls addresses')
    parser.add_argument(
        'jv_s', metavar='JVS', type=str,
        help='GNU assembler file with jump vector to system calls')
    args = parser.parse_args()

    sc = StringIO()
    jv = StringIO()

    with open(args.api) as api:
        doit(api, sc, jv)

    with open(args.sc_lds, 'w') as f:
        f.write(sc.getvalue())

    with open(args.jv_s, 'w') as f:
        f.write(jv.getvalue())
