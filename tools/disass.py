#!/usr/bin/env python3

import argparse
import subprocess
import pygments
import re

from bisect import bisect
from collections import namedtuple, defaultdict

from pygments import lexers
from pygments import formatters

Reloc = namedtuple('Reloc', 'sect addr typ sym')
Symbol = namedtuple('Symbol', 'sect addr name')
SourceLine = namedtuple('SourceLine', 'path num')
DisassLine = namedtuple('DisassLine', 'addr code insn')

c_lexer = lexers.get_lexer_by_name('c')
formatter = formatters.get_formatter_by_name('terminal16m', style='dracula')
reloc_addr = re.compile(
  r'([0-9a-f]+ [0-9a-f]+ )?<([^+-]+)([+-]0x[0-9a-f]+)?>')
symbol_addr = re.compile(r'<([^+-]+)([+-]0x[0-9a-f]+)?>')


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


def format_symbol(name, addr):
    if addr == 0:
        return '<{}>'.format(name)
    if addr < 0:
        return '<{}{}>'.format(name, hex(addr))
    return '<{}+{}>'.format(name, hex(addr))


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

    def _dump_source_line(self, path, num):
        srcline = self._reader.get(path, num)
        srcfile = path.split('/')[-1]

        print()
        print(f'{srcfile:>27}:{num:<3} {srcline}')
        print()

    def _rewrite_operand(self, insn, rel):
        if rel:
            if m := reloc_addr.search(insn):
                try:
                    addend = int(m.group(1).split()[0], 16)
                except AttributeError:
                    addend = 0
                if addend == rel.addr or addend == 0:
                    target = f'<{rel.sym}>'
                else:
                    target = f'<{rel.sym}+0x{addend:x}>'
                insn = insn.replace(m.group(0), target)

        if m := reloc_addr.search(insn):
            symname = m.group(2)
            try:
                symaddr = int(m.group(3), 16)
            except TypeError:
                symaddr = 0
            if target := self._info.preceding_symbol(symname, symaddr):
                insn = insn.replace(m.group(0), format_symbol(*target))

        elif m := symbol_addr.search(insn):
            symname = m.group(1)
            try:
                symaddr = int(m.group(2), 16)
            except TypeError:
                symaddr = 0
            if target := self._info.preceding_symbol(symname, symaddr):
                insn = insn.replace(m.group(0), format_symbol(*target))

        return insn

    def _dump_disass_line(self, addr, code, insn):
        size = len(code.split() * 2)
        rel = self._info.find_reloc('.text', addr, addr + size)

        try:
            mnemonic, operands = insn.split(maxsplit=1)
        except ValueError:
            mnemonic, operands = insn, ''

        operands = [self._rewrite_operand(op, rel)
                    for op in operands.split(',')]
        operands = ','.join(operands)

        print(f'{addr:4x}:\t{code}\t{mnemonic}\t{operands}')

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
