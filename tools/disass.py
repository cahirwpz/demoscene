#!/usr/bin/env python3

import argparse
import subprocess
import pygments

from bisect import bisect
from collections import namedtuple, defaultdict

from pygments import lexers
from pygments import formatters

from pyparsing import (Word, Char, Literal, Opt, ZeroOrMore, OneOrMore, Regex,
                       Suppress, Combine, White, Group)
from pyparsing.exceptions import ParseException


Reloc = namedtuple('Reloc', 'sect addr typ sym')
Symbol = namedtuple('Symbol', 'sect addr name')
SourceLine = namedtuple('SourceLine', 'path num')
DisassLine = namedtuple('DisassLine', 'addr code insn')


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
                        (DataReg | AddrReg) + (WordSize | LongSize) + RBrace)
# absolute short
AbsoluteShort = Combine(Number + WordSize)
# absolute long
AbsoluteLong = Label | Combine(Number + Opt(LongSize))
# program counter with displacement
PCDisp = Group(Label + Combine(LBrace + PCReg + RBrace))
# program counter with index
PCIndex = Group(LBrace + Opt(Label | Number) +
                Combine(Opt(Comma) + PCReg + Comma +
                        DataReg + (WordSize | LongSize) + RBrace))
# immediate
Immediate = Combine(Hash + Number)
# register list (movem)
RegRange = (AddrReg + Minus + AddrReg) | (DataReg + Minus + DataReg)
RegListItem = RegRange | AddrReg | DataReg
RegList = Combine(RegRange | RegListItem + OneOrMore(Char('/') + RegListItem))

Operand = (IndirectPreDec | IndirectPostInc | IndirectIndex | Indirect |
           IndirectDisp | PCDisp | PCIndex | AbsoluteShort | AbsoluteLong |
           Immediate | RegList | DataReg | AddrReg)
Operands = Operand + ZeroOrMore(Suppress(Comma) + Operand)


class InsnCost:
    OperandCost = {
            # byte/word, long, lea
            DataReg: (0, 0, 0),
            AddrReg: (0, 0, 0),
            Indirect: (4, 8, 4),
            IndirectPostInc: (4, 8, 0),
            IndirectPreDec: (6, 10, 0),
            IndirectDisp: (8, 12, 8),
            IndirectIndex: (10, 14, 12),
            AbsoluteLong: (12, 16, 12),
            AbsoluteShort: (8, 12, 8),
            PCDisp: (8, 12, 8),
            PCIndex: (10, 14, 12),
            Immediate: (4, 8, 0),
            Label: (12, 16, 12)}

    def __init__(self):
        pass

    def operand_cost(self, base, kind, operand):
        for parser, cost in self.OperandCost.items():
            try:
                parser.parse_string(operand, parse_all=True)
            except ParseException:
                continue
            if kind in 'sbw':
                return base + cost[0]
            if kind == 'l':
                return base + cost[1]
            if kind == 'lea':
                return base + cost[2]
        raise RuntimeError(operand)

    def parsed_as(self, operand, parser):
        try:
            return bool(parser.parse_string(operand, parse_all=True))
        except ParseException:
            return False

    def is_areg(self, operand):
        return self.parsed_as(operand, AddrReg)

    def is_dreg(self, operand):
        return self.parsed_as(operand, DataReg)

    def cycles(self, length, mnemonic, operands):
        try:
            mnemonic, size = mnemonic.split('.', maxsplit=1)
        except ValueError:
            size = ''

        if len(operands) == 2:
            o0, o1 = operands
        elif len(operands) == 1:
            o0, o1 = operands[0], ''
        else:
            o0, o1 = '', ''

        cost = None
        short = size in 'bsw'

        if mnemonic in ['add', 'adda', 'sub', 'suba', 'and', 'or', 'eor']:
            if self.is_areg(o1):
                return self.operand_cost(8 if short else 6, size, o0)
            if self.is_dreg(o1):
                return self.operand_cost(4 if short else 6, size, o0)
            if self.is_dreg(o0):
                return self.operand_cost(8 if short else 12, size, o0)

        if mnemonic in ['cmp', 'cmpa']:
            if self.is_areg(o1):
                return self.operand_cost(6, size, o0)
            if self.is_dreg(o1):
                return self.operand_cost(4 if short else 6, size, o0)

        if mnemonic in ['muls', 'mulu', 'divs', 'divu']:
            base = {'muls': 70, 'mulu': 70, 'divs': 158, 'divu': 140}
            cost = self.operand_cost(base[mnemonic], size, o0)
            return f'max:{cost}'

        if mnemonic in ['addi', 'subi', 'andi', 'eori', 'ori']:
            if self.is_dreg(o1):
                return 8 if short else 16
            return self.operand_cost(12 if short else 20, size, o1)

        if mnemonic in ['cmpi']:
            if self.is_dreg(o1):
                return 8 if short else 14
            return self.operand_cost(8 if short else 12, size, o1)

        if mnemonic in ['addq', 'subq']:
            if self.is_dreg(o1):
                return 4 if short else 8
            if self.is_areg(o1):
                return 8
            return self.operand_cost(8 if short else 12, size, o1)

        if mnemonic in ['lea']:
            return self.operand_cost(0, 'lea', o0)

        if mnemonic in ['clr', 'neg', 'negx', 'not']:
            if self.is_dreg(o0):
                return 4 if short else 6
            return self.operand_cost(8 if short else 12, size, o0)

        if mnemonic in ['tst']:
            return self.operand_cost(4, size, o0)

        if mnemonic in ['bchg', 'bset']:
            dynamic = self.is_dreg(o0)
            return self.operand_cost(8 if dynamic else 12, 'b', o1)

        if mnemonic in ['btst']:
            dynamic = self.is_dreg(o0)
            if not self.is_dreg(o1):
                return self.operand_cost(4 if dynamic else 8, 'b', o1)
            return 6 if dynamic else 10

        if mnemonic in ['bclr']:
            dynamic = self.is_dreg(o0)
            if self.is_dreg(o1):
                return self.operand_cost(8 if dynamic else 12, 'b', o1)
            return 10 if dynamic else 14

        if mnemonic in ['asr', 'asl', 'lsr', 'lsl', 'ror', 'rol', 'roxr',
                        'roxl']:
            if self.is_dreg(o1):
                return (6 if short else 8) + 2 * int(o0[1:])
            return self.operand_cost(8, size, o0)

        if mnemonic in ['nop', 'swap', 'ext', 'moveq']:
            return 4

        if mnemonic in ['rts', 'link']:
            return 16

        if mnemonic in ['bra']:
            return 10

        if mnemonic in ['bsr']:
            return 18

        if mnemonic.startswith('b'):
            return 't:10/f:8' if short else 't:10/f:12'

        if mnemonic.startswith('s'):
            if self.is_dreg(o1):
                return 4 if short else 6
            return self.operand_cost(8, 'b', o0)

        if mnemonic == 'dbf':
            return 't:10/f:14'

        # print(mnemonic, [self._operand_cost(size, op) for op in operands])


class SourceReader:
    def __init__(self):
        self._files = {}
        self._c_lexer = lexers.get_lexer_by_name('c')
        self._formatter = formatters.get_formatter_by_name('terminal16m',
                                                           style='dracula')

    def get(self, path, line):
        if path not in self._files:
            data = open(path, 'r').read()
            data = pygments.highlight(data, self._c_lexer, self._formatter)
            self._files[path] = data.split('\n')
        return self._files[path][int(line) - 1]


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
        self._cost = InsnCost()

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

    def _dump_source_line(self, path, num):
        srcline = self._reader.get(path, num)
        srcfile = path.split('/')[-1]

        print()
        print(f'{srcfile:>27}:{num:<3} {srcline}')
        print()

    def _parse_operands(self, s):
        if not s:
            return []
        try:
            operands = Operands.parse_string(s, parse_all=True)
            return operands.as_list()
        except ParseException as ex:
            return s

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
            if 'pc' in o2:
                return o0 + self._rewrite_operand(o1, rel) + o2
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

        if (mnemonic.startswith('.') or 'out of bounds' in args
                or operands == args):
            print(f'{addr:4x}:\t{code}\t{mnemonic}\t{args}')
            return

        operands = [self._rewrite_operand(op, rel) for op in operands]

        count = self._cost.cycles(size, mnemonic, operands)
        count = f'; {count}' if count else '; ?'

        operands = ','.join(operands)

        print(f'{addr:4x}:\t{code}\t{mnemonic}\t{operands:36}{count}')

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
