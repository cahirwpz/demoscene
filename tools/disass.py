#!/usr/bin/env python3

import argparse
import subprocess
import pygments

from bisect import bisect
from collections import namedtuple, defaultdict, UserString, UserList

from pygments import lexers
from pygments import formatters

from pyparsing import (Word, Char, Literal, Opt, ZeroOrMore, OneOrMore, Regex,
                       Suppress, Combine, White, Group)
from pyparsing.exceptions import ParseException


Reloc = namedtuple('Reloc', 'sect addr typ sym')
Symbol = namedtuple('Symbol', 'sect addr name')
SourceLine = namedtuple('SourceLine', 'path num')
DisassLine = namedtuple('DisassLine', 'addr code insn')

OpDataReg = type('OpDataReg', (UserString,), dict())
OpAddrReg = type('OpAddrReg', (UserString,), dict())
OpIndirect = type('OpIndirect', (UserString,), dict())
OpIndirectPreDec = type('OpIndirectPreDec', (UserString,), dict())
OpIndirectPostInc = type('OpIndirectPostInc', (UserString,), dict())
OpIndirectDisp = type('OpIndirectDisp', (UserString,), dict())
OpIndirectIndex = type('OpIndirectIndex', (UserString,), dict())
OpAbsoluteShort = type('OpAbsoluteShort', (UserString,), dict())
OpAbsoluteLong = type('OpAbsoluteLong', (UserString,), dict())
OpPCDisp = type('OpPCDisp', (UserString,), dict())
OpPCIndex = type('OpPCIndex', (UserString,), dict())
OpImmediate = type('OpImmediate', (UserString,), dict())


class OpRegList(UserList):
    def __str__(self):
        return '/'.join(map(str, self.data))

    def regcount(self):
        cnt = 0
        for rl in self.data:
            try:
                first, last = rl.split('-')
                if last == 'sp':
                    last = 'a7'
                assert first[0] == last[0]
                cnt += int(last[1]) - int(first[1]) + 1
            except ValueError:
                cnt += 1
        return cnt


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
DataReg.set_parse_action(lambda t: OpDataReg(t[0]))
# address register direct
AddrReg = Combine(Char('a') + RegNum | StackReg)
AddrReg.set_parse_action(lambda t: OpAddrReg(t[0]))
# address register indirect
Indirect = Combine(LBrace + AddrReg + RBrace)
Indirect.set_parse_action(lambda t: OpIndirect(t[0]))
# address register indirect with pre-decrement
IndirectPreDec = Combine(Minus + Indirect)
IndirectPreDec.set_parse_action(lambda t: OpIndirectPreDec(t[0]))
# address register indirect with post-increment
IndirectPostInc = Combine(Indirect + Plus)
IndirectPostInc.set_parse_action(lambda t: OpIndirectPostInc(t[0]))
# address register indirect with displacement
IndirectDisp = Combine(Number + Indirect)
IndirectDisp.set_parse_action(lambda t: OpIndirectDisp(t[0]))
# address register indeirect with index
IndirectIndex = Combine(LBrace + Opt(Number + Comma) + AddrReg + Comma +
                        (DataReg | AddrReg) + (WordSize | LongSize) + RBrace)
IndirectIndex.set_parse_action(lambda t: OpIndirectIndex(t[0]))
# absolute short
AbsoluteShort = Combine(Number + WordSize)
AbsoluteShort.set_parse_action(lambda t: OpAbsoluteShort(t[0]))
# absolute long
AbsoluteLong = Combine(Number + Opt(LongSize))
AbsoluteLong.set_parse_action(lambda t: OpAbsoluteLong(t[0]))
# program counter with displacement
PCDisp = Group(Label + Combine(LBrace + PCReg + RBrace))
# program counter with index
PCIndex = Group(LBrace + Opt(Label | Number) +
                Combine(Opt(Comma) + PCReg + Comma +
                        DataReg + (WordSize | LongSize) + RBrace))
# immediate
Immediate = Combine(Hash + Number)
Immediate.set_parse_action(lambda t: OpImmediate(t[0]))
# register list (movem)
RegRange = Combine((AddrReg + Minus + AddrReg) | (DataReg + Minus + DataReg))
RegListItem = RegRange | AddrReg | DataReg
RegList = Group(RegListItem + OneOrMore(Suppress(Char('/')) + RegListItem) |
                RegRange)
RegList.set_parse_action(lambda t: OpRegList(t[0]))

Operand = (IndirectPreDec | IndirectPostInc | IndirectIndex | Indirect |
           IndirectDisp | PCDisp | PCIndex | Label | AbsoluteShort |
           AbsoluteLong | Immediate | RegList | DataReg | AddrReg)
Operands = Operand + ZeroOrMore(Suppress(Comma) + Operand)


class InsnCost:
    OperandCost = {
            OpDataReg: (0, 0),
            OpAddrReg: (0, 0),
            OpIndirect: (4, 8),
            OpIndirectPostInc: (4, 8),
            OpIndirectPreDec: (6, 10),
            OpIndirectDisp: (8, 12),
            OpIndirectIndex: (10, 14),
            OpAbsoluteShort: (8, 12),
            OpAbsoluteLong: (12, 16),
            OpPCDisp: (8, 12),
            OpPCIndex: (10, 14),
            OpImmediate: (4, 8),
        }

    MoveDest = {
            OpDataReg: 0,
            OpAddrReg: 1,
            OpIndirect: 2,
            OpIndirectPostInc: 3,
            OpIndirectPreDec: 4,
            OpIndirectDisp: 5,
            OpIndirectIndex: 6,
            OpAbsoluteShort: 7,
            OpAbsoluteLong: 8,
        }

    MoveShortCost = {
            OpDataReg: (4, 4, 8, 8, 8, 12, 14, 12, 16),
            OpAddrReg: (4, 4, 8, 8, 8, 12, 14, 12, 16),
            OpIndirect: (8, 8, 12, 12, 12, 16, 18, 16, 20),
            OpIndirectPostInc: (8, 8, 12, 12, 12, 16, 18, 16, 20),
            OpIndirectPreDec: (10, 10, 14, 14, 14, 18, 20, 18, 22),
            OpIndirectDisp: (12, 12, 16, 16, 16, 20, 22, 20, 24),
            OpIndirectIndex: (14, 14, 18, 18, 18, 22, 24, 22, 26),
            OpAbsoluteShort: (12, 12, 16, 16, 16, 20, 22, 20, 24),
            OpAbsoluteLong: (16, 16, 20, 20, 20, 24, 26, 24, 28),
            OpPCDisp: (12, 12, 16, 16, 16, 20, 22, 20, 24),
            OpPCIndex: (14, 14, 18, 18, 18, 22, 24, 22, 26),
            OpImmediate: (8, 8, 12, 12, 12, 16, 18, 16, 20),
        }

    MoveLongCost = {
            OpDataReg: (4, 4, 12, 12, 12, 16, 18, 16, 20),
            OpAddrReg: (4, 4, 12, 12, 12, 16, 18, 16, 20),
            OpIndirect: (12, 12, 20, 20, 20, 24, 26, 24, 28),
            OpIndirectPostInc: (12, 12, 20, 20, 20, 24, 26, 24, 28),
            OpIndirectPreDec: (14, 14, 22, 22, 22, 26, 28, 26, 30),
            OpIndirectDisp: (16, 16, 24, 24, 24, 28, 30, 28, 32),
            OpIndirectIndex: (18, 18, 26, 26, 26, 30, 32, 30, 34),
            OpAbsoluteShort: (16, 16, 24, 24, 24, 28, 30, 28, 32),
            OpAbsoluteLong: (20, 20, 28, 28, 28, 32, 34, 32, 36),
            OpPCDisp: (16, 16, 24, 24, 24, 28, 30, 28, 32),
            OpPCIndex: (18, 18, 26, 26, 26, 30, 32, 30, 34),
            OpImmediate: (12, 12, 20, 20, 20, 24, 26, 24, 28),
        }

    InsnCost = {
            'jmp': {
                OpIndirect: 8,
                OpIndirectDisp: 10,
                OpIndirectIndex: 14,
                OpAbsoluteShort: 10,
                OpAbsoluteLong: 12,
                OpPCDisp: 10,
                OpPCIndex: 14,
            },
            'jsr': {
                OpIndirect: 16,
                OpIndirectDisp: 18,
                OpIndirectIndex: 22,
                OpAbsoluteShort: 18,
                OpAbsoluteLong: 20,
                OpPCDisp: 18,
                OpPCIndex: 22,
            },
            'lea': {
                OpIndirect: 4,
                OpIndirectDisp: 8,
                OpIndirectIndex: 12,
                OpAbsoluteShort: 8,
                OpAbsoluteLong: 12,
                OpPCDisp: 8,
                OpPCIndex: 12,
            },
            'pea': {
                OpIndirect: 4,
                OpIndirectDisp: 8,
                OpIndirectIndex: 12,
                OpAbsoluteShort: 8,
                OpAbsoluteLong: 12,
                OpPCDisp: 8,
                OpPCIndex: 12,
            }
        }

    MovemCost = {
            OpIndirect: (12, 8),
            OpIndirectPostInc: (12, None),
            OpIndirectPreDec: (None, 8),
            OpIndirectDisp: (16, 12),
            OpIndirectIndex: (18, 14),
            OpAbsoluteShort: (16, 12),
            OpAbsoluteLong: (20, 16),
            OpPCDisp: (16, None),
            OpPCIndex: (16, None),
        }

    MiscCost = {
            'nop': 4,
            'swap': 4,
            'ext': 4,
            'moveq': 4,
            'exg': 6,
            'rts': 16,
            'link': 16,
            'bra': 10,
            'bsr': 18,
            'rte': 20,
            'unlk': 12,
        }

    def operand_cost(self, base, kind, operand):
        for tokenCls, cost in self.OperandCost.items():
            if isinstance(operand, tokenCls):
                if kind in 'sbw':
                    return base + cost[0]
                if kind == 'l':
                    return base + cost[1]
        raise RuntimeError(operand, type(operand))

    def is_areg(self, op):
        return isinstance(op, OpAddrReg)

    def is_dreg(self, op):
        return isinstance(op, OpDataReg)

    def is_imm(self, op):
        return isinstance(op, OpImmediate)

    def is_reglist(self, op):
        return isinstance(op, OpRegList)

    def is_direct(self, op):
        return self.is_areg(op) or self.is_dreg(op) or self.is_imm(op)

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

        short = size in 'bsw'

        if mnemonic in ['move', 'movea']:
            if short:
                return self.MoveShortCost[type(o0)][self.MoveDest[type(o1)]]
            return self.MoveLongCost[type(o0)][self.MoveDest[type(o1)]]

        if mnemonic in ['add', 'adda', 'sub', 'suba', 'and', 'or']:
            l_cost = 8 if self.is_direct(o0) else 6
            if self.is_areg(o1):
                return self.operand_cost(8 if short else l_cost, size, o0)
            if self.is_dreg(o1):
                return self.operand_cost(4 if short else l_cost, size, o0)
            if self.is_dreg(o0):
                return self.operand_cost(8 if short else 12, size, o0)

        if mnemonic in ['eor']:
            if self.is_dreg(o1):
                return self.operand_cost(4 if short else 8, size, o0)
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

        if mnemonic in ['addi', 'subi', 'eori', 'ori']:
            if self.is_dreg(o1):
                return 8 if short else 16
            return self.operand_cost(12 if short else 20, size, o1)

        if mnemonic in ['andi']:
            if self.is_dreg(o1):
                return 8 if short else 14
            return self.operand_cost(12 if short else 20, size, o1)

        if mnemonic in ['cmpi']:
            if self.is_dreg(o1):
                return 8 if short else 14
            return self.operand_cost(8 if short else 12, size, o1)

        if mnemonic in ['addq']:
            if self.is_dreg(o1) or self.is_areg(o1):
                return 4 if short else 8
            return self.operand_cost(8 if short else 12, size, o1)

        if mnemonic in ['subq']:
            if self.is_dreg(o1):
                return 4 if short else 8
            if self.is_areg(o1):
                return 8
            return self.operand_cost(8 if short else 12, size, o1)

        if mnemonic in ['clr', 'neg', 'negx', 'not']:
            if self.is_dreg(o0):
                return 4 if short else 6
            return self.operand_cost(8 if short else 12, size, o0)

        if mnemonic in ['nbcd']:
            if self.is_dreg(o0):
                return 4
            return self.operand_cost(6, 'b', o0)

        if mnemonic in ['tas']:
            if self.is_dreg(o0):
                return 4
            return self.operand_cost(14, 'b', o0)

        if mnemonic in ['tst']:
            return self.operand_cost(4, size, o0)

        if mnemonic in ['bchg', 'bset']:
            dynamic = self.is_dreg(o0)
            return self.operand_cost(8 if dynamic else 12, 'b', o1)

        if mnemonic in ['bclr']:
            dynamic = self.is_dreg(o0)
            if self.is_dreg(o1):
                return self.operand_cost(8 if dynamic else 12, 'b', o1)
            return 10 if dynamic else 14

        if mnemonic in ['btst']:
            dynamic = self.is_dreg(o0)
            if not self.is_dreg(o1):
                return self.operand_cost(4 if dynamic else 8, 'b', o1)
            return 6 if dynamic else 10

        if mnemonic in ['asr', 'asl', 'lsr', 'lsl', 'ror', 'rol', 'roxr',
                        'roxl']:
            if self.is_dreg(o1):
                return (6 if short else 8) + 2 * int(o0[1:])
            return self.operand_cost(8, size, o0)

        if mnemonic in ['jmp', 'jsr', 'lea', 'pea']:
            return self.InsnCost[mnemonic][type(o0)]

        if mnemonic in ['movep']:
            return 16 if short else 24

        if mnemonic in ['movem']:
            if self.is_reglist(o1):
                cnt = o1.regcount() * (4 if short else 8)
                return self.MovemCost[type(o0)][0] + cnt
            cnt = o0.regcount() * (4 if short else 8)
            return self.MovemCost[type(o1)][1] + cnt

        if cost := self.MiscCost.get(mnemonic):
            return cost

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
        self.read_symbols(path)
        self.read_relocs(path)
        self.make_indices()

    def read_symbols(self, path):
        output = subprocess.run(
                ['m68k-amigaos-objdump', '-G', path], capture_output=True)
        secmap = {'FUN': '.text', 'STSYM': '.data', 'LCSYM': '.bss'}

        symbols = []
        regs = defaultdict(dict)

        for line in output.stdout.decode('utf-8').splitlines():
            fs = line.split()
            if len(fs) != 7:
                continue
            if secname := secmap.get(fs[1], None):
                addr, name = int(fs[4], 16), fs[6].split(':')[0]
                symbols.append(Symbol(secname, addr, '_' + name))
            elif fs[1] == 'RSYM':
                line = int(fs[3])
                reg = int(fs[4], 16)
                if reg < 8:
                    reg = f'd{reg}'
                else:
                    reg -= 8
                    reg = f'a{reg}'
                var = fs[6].split(':')[0]
                regs[line][var] = reg

        self._regs = regs
        self._symbols = symbols

    def read_relocs(self, path):
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

        self._relocs = relocs

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

    def local_vars(self, line):
        return self._regs.get(line, dict())


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

        if lvars := self._info.local_vars(num):
            print(' ' * 31, '; variable',
                  ', '.join(f'`{n}` in {r}' for n, r in lvars.items()))

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
            return OpAbsoluteLong('<{}>'.format(name))
        if addr < 0:
            return OpAbsoluteLong('<{}{}>'.format(name, hex(addr)))
        return OpAbsoluteLong('<{}+{}>'.format(name, hex(addr)))

    def _rewrite_operand(self, operand, rel):
        if type(operand) is not list:
            return operand

        if len(operand) == 2:
            o0, o1 = operand
            if o1 == '(pc)':
                return OpPCDisp(str(self._rewrite_operand(o0, rel)) + o1)
            return self._rewrite_label(int(o0, 16), o1, 0, rel)
        elif len(operand) == 3:
            o0, o1, o2 = operand
            if 'pc' in o2:
                return OpPCIndex(o0 + str(self._rewrite_operand(o1, rel)) + o2)
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

        operands = ','.join(map(str, operands))

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
