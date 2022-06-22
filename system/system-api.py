#!/usr/bin/env python3

import argparse
from contextlib import redirect_stdout
from collections import namedtuple, OrderedDict
from io import StringIO


StabType = namedtuple('StabType', ['name', 'idx', 'desc'])


TYPES = OrderedDict([
    ('int', StabType('int', 1, 'r1;-2147483648;2147483647;')),
    ('u_int', StabType('unsigned int', 2, 'r1;0;-1;')),
    ('short', StabType('short int', 3, 'r1;-32768;32767;')),
    ('u_short', StabType('short unsigned int', 4, 'r1;0;65535;')),
    ('char', StabType('signed char', 5, 'r1;-128;127;')),
    ('u_char', StabType('unsigned char', 6, 'r1;0;255;')),
    ('void', StabType('void', 7, '7')),
])


SIZEOF = {
    'int': 4,
    'u_int': 4,
    'short': 2,
    'u_short': 2,
    'char': 1,
    'u_char': 1,
}


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
            shvars.append([fs[1], fs[2]])
        else:
            raise SystemExit(f'line {i+1}: syntax error: {line}')

    syscalls = [(name, i * 6 + 0xc0) for i, name in enumerate(syscalls)]
    shvars_sorted = sorted(shvars, key=lambda x: -SIZEOF.get(x[1]))
    shvars = []

    addr = 0x400
    for name, typ in shvars_sorted:
        addr -= SIZEOF.get(typ)
        shvars.append((name, addr, typ))

    with redirect_stdout(sc):
        print("#include <asm.h>")
        print("#include <stab.h>")
        print()
        print('\t.stabs\t"syscall.S",N_SO,0,0,0')
        for name, idx, desc in TYPES.values():
            print(f'\t.stabs\t"{name}:t{idx}={desc}",'
                  f' N_LSYM, 0, 0, 0')
        print()
        print('\t# system calls')
        for name, addr in syscalls:
            print(f'\t.set\t_L(_{name}), 0x{addr:x}')
            print(f'\t.stabs\t__STRING(_L(_{name})),'
                  f' N_ABS|N_EXT, 0, 0, 0x{addr:x}')
        print()
        print('\t# shared variables')
        for name, addr, typ in shvars:
            print(f'\t.set\t_L({name}), 0x{addr:x}')
            print(f'\t.size\t_L({name}), {SIZEOF[typ]}')
            print(f'\t.stabs\t__STRING(_L({name})), '
                  f'N_ABS|N_EXT, 0, 0, 0x{addr:x}')
            print(f'\t.stabs\t\"{name}:G{TYPES[typ].idx}\",'
                  f' N_GSYM, 0, 0, 0x{addr:x}')
        print()
        print("# vim: ft=gas:ts=8:sw=8:noet")

    with redirect_stdout(jv):
        print("#include <asm.h>")
        print()
        print(f"ENTRY(JumpTable)")
        for name, _ in syscalls:
            print(f"\tjmp\t_L({name})")
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
        'sc_s', metavar='SCS', type=str,
        help='GNU as file with system calls & shared variables addresses')
    parser.add_argument(
        'jv_s', metavar='JVS', type=str,
        help='GNU as file with jump vector to system calls')
    args = parser.parse_args()

    sc = StringIO()
    jv = StringIO()

    with open(args.api) as api:
        doit(api, sc, jv)

    with open(args.sc_s, 'w') as f:
        f.write(sc.getvalue())

    with open(args.jv_s, 'w') as f:
        f.write(jv.getvalue())
