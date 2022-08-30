#!/usr/bin/env python3

import argparse
import subprocess
import pygments
import re

from bisect import bisect
from collections import namedtuple, defaultdict

from pygments import lexers
from pygments import formatters

from pyparsing import (
        Word, Char, Literal, Opt, ZeroOrMore, Empty, Regex,
        Suppress, Combine, White, Group)
from pyparsing.exceptions import ParseException


Reloc = namedtuple('Reloc', 'sect addr typ sym')
Symbol = namedtuple('Symbol', 'sect addr name')
SourceLine = namedtuple('SourceLine', 'path num')
DisassLine = namedtuple('DisassLine', 'addr code insn')

c_lexer = lexers.get_lexer_by_name('c')
formatter = formatters.get_formatter_by_name('terminal16m', style='dracula')
reloc_addr = re.compile(
  r'([0-9a-f]+\s+)?([0-9a-f]+\s+)?<([^+-]+)([+-]0x[0-9a-f]+)?>')
symbol_addr = re.compile(r'<([^+-]+)([+-]0x[0-9a-f]+)?>')


Dot = Char('.')
Hash = Char('#')
Comma = Char(',')
LBrace = Char('(')
RBrace = Char(')')
Plus = Char('+')
Minus = Char('-')
RegNum = Char('01234567')
WordSize = Literal('.w')
LongSize = Literal('.l')
StackReg = Literal('sp')
PCReg = Literal('pc')
Digits = Word('0123456789')
HexDigits = Word('0123456789abcdef')
Number = Opt(Minus) + Digits
HexNumber = Combine((Suppress(Plus) | Minus) +
                    Suppress(Literal('0x')) + HexDigits)
Section = Literal('.text') | Literal('.data') | Literal('.bss')
Label = Group(
          Opt(HexDigits + Suppress(White())) +
          Opt(Suppress(HexDigits + White())) +
          Suppress(Char('<')) + (Section | Regex('[A-Za-z0-9_]+')) +
          Opt(HexNumber) + Suppress(Char('>')))

# data register direct
DataReg = Combine(Char('d') + RegNum)
# address register direct
AddrReg = Combine(Char('a') + RegNum | StackReg)
# address register indirect
Indirect = Combine(LBrace + AddrReg + RBrace)
# address register indirect with pre-decrement
IndirectPreDec = Combine(Minus + Indirect)
# address register indirect with post-increment
IndirectPostInc = Combine(Indirect + Plus)
# address register indirect with displacement
IndirectDisp = Combine(Number + Indirect)
# address register indeirect with index
IndirectIndex = Combine(LBrace + Opt(Number + Comma) + AddrReg + Comma +
                        DataReg + (WordSize | LongSize) + RBrace)
# absolute short
AbsoluteShort = Combine(Number + WordSize)
# absolute long
AbsoluteLong = Combine(Number + Opt(LongSize))
# program counter with displacement
PCDisp = Group(Label + Combine(LBrace + PCReg + RBrace))
# program counter with index
PCIndex = Combine(LBrace + Opt((Label | Number) + Comma) + PCReg + Comma +
                  DataReg + (WordSize | LongSize) + RBrace)
# immediate
Immediate = Combine(Hash + Number)
# register list (movem)
RegRange = (AddrReg + Minus + AddrReg) | (DataReg + Minus + DataReg)
RegListItem = RegRange | AddrReg | DataReg
RegList = Combine(RegListItem + ZeroOrMore(Char('/') + RegListItem))

Operand = (IndirectPreDec | IndirectPostInc | IndirectIndex | Indirect |
           IndirectDisp | PCDisp | PCIndex |
           Label('label') | AbsoluteShort | AbsoluteLong | Immediate |
           RegList | DataReg | AddrReg)
Operands = Operand + ZeroOrMore(Suppress(Comma) + Operand)


class SourceReader:
    def __init__(self):
        self.files = {}

    def get(self, path, line):
        if path not in self.files:
            data = open(path, 'r').read()
            data = pygments.highlight(data, c_lexer, formatter)
            self.files[path] = data.split('\n')
        return self.files[path][int(line) - 1]


class ObjectInfo:
    def __init__(self, path):
        self._symbols = self.read_symbols(path)
        self._relocs = self.read_relocs(path)

        self.make_indices()

    @staticmethod
    def read_symbols(path):
        output = subprocess.run(
                ['m68k-amigaos-objdump', '-G', path], capture_output=True)
        secmap = {'FUN': '.text', 'STSYM': '.data', 'LCSYM': '.bss'}

        symbols = []

        for line in output.stdout.decode('utf-8').splitlines():
            fs = line.split()
            if len(fs) != 7:
                continue
            if secname := secmap.get(fs[1], None):
                addr, name = int(fs[4], 16), fs[6].split(':')[0]
                symbols.append(Symbol(secname, addr, '_' + name))

        return symbols

    @staticmethod
    def read_relocs(path):
        output = subprocess.run(
                ['m68k-amigaos-objdump', '-r', path], capture_output=True)

        relocs = []
        sect = None

        for line in output.stdout.decode('utf-8').splitlines():
            fs = line.split()
            if not fs:
                continue
            if 'RELOCATION' == fs[0]:
                sect = fs[3][1:-2]
            if 'RELOC' in fs[1]:
                val, typ, sym = int(fs[0], 16), fs[1], fs[2]
                relocs.append(Reloc(sect, val, typ, sym))

        return relocs

    def make_indices(self):
        index = defaultdict(list)
        for sym in self._symbols:
            index[sym.sect].append(sym)
        for sect in index:
            index[sect] = sorted(index[sect], key=lambda s: s.addr)
        self._symbols_by_section = index
        self._symbols_by_name = {sym.name: sym for sym in self._symbols}

        index = defaultdict(list)
        for rel in self._relocs:
            index[rel.sect].append(rel)
        for sect in index:
            index[sect] = sorted(index[sect], key=lambda s: s.addr)
        self._relocs_by_section = index

    def preceding_symbol(self, name, addr):
        if symbol := self._symbols_by_name.get('_' + name, None):
            name = symbol.sect
            addr = addr + symbol.addr

        if symbols := self._symbols_by_section.get(name, None):
            addresses = [x.addr for x in symbols]
            i = bisect(addresses, addr) - 1
            sym = symbols[i]
            return sym.name, addr - sym.addr

    def find_reloc(self, sect, start, end):
        if relocs := self._relocs_by_section.get(sect, None):
            addresses = [x.addr for x in relocs]
            i = bisect(addresses, start)
            if i >= len(relocs):
                reloc = relocs[-1]
            else:
                reloc = relocs[i]
            if reloc.addr >= start and reloc.addr < end:
                return reloc

    def find_symbol(self, name):
        return self._symbols_by_name.get(name, None)


class Disassembler:
    def __init__(self, path):
        self._path = path
        self._reader = SourceReader()
        self._info = ObjectInfo(path)

    def _read(self, func):
        output = subprocess.run(
                ['m68k-amigaos-objdump', '-d', '-l', '-C',
                 '--source-comment=;', '--insn-width=8', self._path],
                capture_output=True)

        inside = func is None
        lines = []

        for line in output.stdout.decode('utf-8').splitlines():
            if line.startswith(';'):
                continue

            if func and line.endswith(':'):
                inside = line[:-1] == func
            if not inside:
                continue

            if line.startswith('/'):
                path, line = line.split(':')
                lines.append(SourceLine(path, int(line)))
            else:
                try:
                    addr, code, insn = line.split('\t')
                except ValueError:
                    continue

                addr = int(addr[:-1], 16)
                lines.append(DisassLine(addr, code, insn))

        return lines

    def _count_cycles(self, length, mnemonic, operands):
        # print([mnemonic] + operands)
        pass

    def _dump_source_line(self, path, num):
        srcline = self._reader.get(path, num)
        srcfile = path.split('/')[-1]

        print()
        print(f'{srcfile:>27}:{num:<3} {srcline}')
        print()

    def _parse_operands(self, s):
        if not s:
            return None
        try:
            operands = Operands.parse_string(s, parse_all=True).as_list()
        except ParseException as ex:
            operands = None
        return operands


    def _rewrite_label(self, addend, name, addr, rel):
        if rel:
            name, addr = rel.sym, addend

        if target := self._info.preceding_symbol(name, addr):
            name, addr = target

        if addr == 0:
            return '<{}>'.format(name)
        if addr < 0:
            return '<{}{}>'.format(name, hex(addr))
        return '<{}+{}>'.format(name, hex(addr))

    def _rewrite_operand(self, operand, rel):
        if type(operand) is not list:
            return operand

        if len(operand) == 2:
            o0, o1 = operand
            if o1 == '(pc)':
                return self._rewrite_operand(o0, rel) + o1
            return self._rewrite_label(int(o0, 16), o1, 0, rel)
        elif len(operand) == 3:
            o0, o1, o2 = operand
            return self._rewrite_label(int(o0, 16), o1, int(o2, 16), rel)
        else:
            raise RuntimeError(operand)

    def _dump_disass_line(self, addr, code, insn):
        size = len(code.split() * 2)
        rel = self._info.find_reloc('.text', addr, addr + size)

        try:
            mnemonic, args = insn.split(maxsplit=1)
        except ValueError:
            mnemonic, args = insn, ''

        operands = self._parse_operands(args)

        if mnemonic.startswith('.') or 'out of bounds' in args or not operands:
            print(f'{addr:4x}:\t{code}\t{mnemonic}\t{args}')
            return

        operands = [self._rewrite_operand(op, rel) for op in operands]

        cycles = self._count_cycles(size, mnemonic, operands)
        operands = ','.join(operands)

        print(f'{addr:4x}:\t{code}\t{mnemonic}\t{operands}\t; {cycles}')

    def disassemble(self, func):
        for line in self._read(func):
            if type(line) == SourceLine:
                self._dump_source_line(*line)
            if type(line) == DisassLine:
                self._dump_disass_line(*line)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Dump function assembly from given file.')
    parser.add_argument('objfile', metavar='OBJFILE', type=str,
                        help='AmigaHunk object file.')
    parser.add_argument('--function', type=str, help='Function name.')
    args = parser.parse_args()

    disass = Disassembler(args.objfile)
    disass.disassemble(args.function)
