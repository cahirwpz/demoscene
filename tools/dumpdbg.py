#!/usr/bin/env python3

import argparse
import logging

from uaedbg.info import DebugInfoReader, Segment


def do_symbol(di, name):
    try:
        sec, addr = di.find_symbol_addr(name)
        print('<{}> at {:08X} in {}'.format(
            name, addr, sec.hunk_type))
    except TypeError:
        print('<{}> not found!'.format(name))


def do_symbol_by_address(di, addr):
    try:
        sec, (name, real_addr) = di.find_symbol(addr)
        print('<{}+{}> at {:08X} in {}'.format(
            name, addr - real_addr, addr, sec.hunk_type))
    except TypeError:
        print('Nothing found at {:08X}!'.format(addr))


def do_line(di, line):
    try:
        sec, addr = di.find_line_addr(line)
        print('<{}> at {:08X} in {}'.format(line, addr, sec.hunk_type))
    except TypeError:
        print('<{}> not found!'.format(line))


def do_function(di, name):
    try:
        sec, fn = di.find_function(name)
        print('<{}> in {}'.format(fn, sec.hunk_type))
    except TypeError:
        print('<{}> not found!'.format(name))


def do_variable(di, name):
    try:
        sec, var = di.find_variable(name)
        print('<{}> in {}'.format(var, sec.hunk_type))
    except TypeError:
        print('<{}> not found!'.format(name))


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Dump contents of HUNK_DEBUG sections in stabs format.')
    parser.add_argument('-d', '--dump', action='store_true',
                        help='Dump debug information on standard output.')
    parser.add_argument('-r', '--relocate', type=int, action='append',
                        help=('An address to relocate section to.'
                              'Must be specified for each section.'))
    parser.add_argument('-s', '--symbol', type=str,
                        help='Find address of a symbol.')
    parser.add_argument('-S', '--symbol-by-address', type=lambda x: int(x, 16),
                        help='Locate symbol by an address.')
    parser.add_argument('-l', '--line', type=str,
                        help='Locate address of source line.')
    parser.add_argument('-f', '--function', type=str,
                        help='Display information about a function.')
    parser.add_argument('-v', '--variable', type=str,
                        help='Display information about a variable.')
    parser.add_argument('hunk', metavar='HUNK', type=str, nargs='+',
                        help='Amiga Hunk file with debug information.')
    args = parser.parse_args()

    logging.basicConfig()

    for path in args.hunk:
        print('Parsing "%s".' % path)
        print('')

        di = DebugInfoReader(path)

        if args.relocate:
            relo_target = [Segment(addr, sec.size)
                           for addr, sec in zip(args.relocate, di.sections)]

            print('Before relocation:')
            for sec in di.sections:
                print(' {}'.format(sec))
            if not di.relocate(relo_target):
                raise SystemExit('Relocation failed!')
            print()

            print('After relocation:')
            for sec in di.sections:
                print(' {}'.format(sec))

        action = False

        if args.symbol:
            do_symbol(di, args.symbol)
            action = True

        if args.symbol_by_address:
            do_symbol_by_address(di, args.symbol_by_address)
            action = True

        if args.line:
            do_line(di, args.line)
            action = True

        if args.function:
            do_function(di, args.function)
            action = True

        if args.variable:
            do_variable(di, args.variable)
            action = True

        if args.dump or not action:
            di.dump()

        print('')
