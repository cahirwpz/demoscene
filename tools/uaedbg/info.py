import os.path

from collections import namedtuple
from collections.abc import Sequence

from pygments import highlight
from pygments.lexers.c_cpp import CLexer
from pygments.styles import get_style_by_name
from pygments.formatters import Terminal256Formatter

from .hunk import ReadHunkFile
from .stabs import (StabInfoParser, TypeDecl, Pointer, ArrayType,
                    StructType, ForwardDecl, FunctionType)
from .state import Registers


TheFormatter = Terminal256Formatter(style=get_style_by_name('monokai'))


Segment = namedtuple('Segment', 'start size')


class SourceLine():
    def __init__(self, source, line):
        self.source = source
        self.line = line

    @property
    def path(self):
        return self.source.path

    def __lt__(self, other):
        if self.source == other.source:
            return self.line < other.line
        return self.source < other.source

    def __repr__(self):
        return '<{}:{}>'.format(self.source, self.line)


class SourceFile(Sequence):
    def __init__(self, path):
        self.path = path
        self.__lines = {}
        self.__code = None

    @property
    def _code(self):
        if self.__code is None:
            with open(self.path) as fh:
                formatted = highlight(fh.read(), CLexer(), TheFormatter)
                self.__code = formatted.splitlines()
        return self.__code

    def __getitem__(self, index):
        return self._code[index]

    def __len__(self):
        return len(self._code)

    def line(self, lineno):
        try:
            return self.__lines[lineno]
        except KeyError:
            sl = SourceLine(self, lineno)
            self.__lines[lineno] = sl
            return sl

    def __str__(self):
        return os.path.basename(self.path)


class TranslationUnit():
    def __init__(self, path):
        self.path = path
        self.types = {}

    def add_type(self, ref, typedecl):
        self.types[ref] = typedecl

    def get_type_name(self, typ):
        while isinstance(typ, int):
            _typ = self.types.get(typ, None)
            if not _typ:
                raise KeyError(str(self), typ)
            if _typ == typ:
                break
            typ = _typ
        if isinstance(typ, TypeDecl):
            typ = typ.name
        elif isinstance(typ, Pointer):
            typ = self.get_type_name(typ.type) + '*'
        elif isinstance(typ, ArrayType):
            typ = self.get_type_name(typ.type) + '[{}]'.format(typ.range.count)
        elif isinstance(typ, StructType):
            fields = [self.get_type_name(field.type) + ' ' + field.name
                      for field in typ.fields]
            typ = 'struct {' + '; '.join(fields) + '; }'
        elif isinstance(typ, ForwardDecl):
            typ = typ.type + ' ' + typ.name
        elif isinstance(typ, FunctionType):
            typ = self.get_type_name(typ.type)
        return str(typ)

    def dump(self):
        for n, typ in sorted(self.types.items()):
            print(n, typ)

    def __str__(self):
        return os.path.basename(self.path)


class Function(namedtuple('Function', 'unit line attr type scopes_')):
    __slots__ = ()

    def add_scope(self, scope):
        if len(scope):
            self.scopes_.append(scope)

    @property
    def scopes(self):
        return sorted(self.scopes_, key=lambda x: x.begin)


class Scope():
    def __init__(self, begin=0, end=0, values=None):
        self.begin = begin
        self.end = end
        self.values = list(values or [])

    def add(self, val):
        self.values.append(val)

    def __len__(self):
        return len(self.values)

    def __repr__(self):
        return 'scope [{:08X} - {:08X}]'.format(self.begin, self.end)


class ScopeStabParser():
    def __init__(self, begin):
        self._begin = begin
        self._nested = []

    def add(self, v):
        self._nested[-1].add(v)

    def new(self, addr):
        self._nested.append(Scope(begin=addr))

    def enter(self, addr):
        self._nested.append(Scope(begin=self._begin + addr))

    def leave(self, addr):
        s = self._nested.pop()
        s.end = self._begin + addr
        return s

    def finish(self, addr):
        s = self._nested.pop()
        assert len(self._nested) == 0
        s.end = addr
        return s


Symbol = namedtuple('Symbol', 'name addr')
ParamReg = namedtuple('ParamReg', 'name line reg type')
ParamStk = namedtuple('ParamStk', 'name line offset type')
Variable = namedtuple('Variable', 'unit line attr type')


class Section():
    def __init__(self, h, start=0, size=0):
        self.hunk = h
        self.start = start
        self.size = size
        self.symbols = {}
        self.lines = {}
        self.functions = {}
        self.variables = {}
        self.units = []

    @property
    def end(self):
        return self.start + self.size

    @property
    def hunk_type(self):
        return self.hunk.type

    def add_unit(self, unit):
        self.units.append(unit)

    def add_line(self, addr, line):
        self.lines[addr] = line

    def add_symbol(self, name, addr):
        # if you have two symbols '_a' and 'a'
        # with the same address, then choose latter one
        if name[0] == '_' and self.symbols.get(name[1:], None) == addr:
            return
        if self.symbols.get('_' + name, None) == addr:
            del self.symbols['_' + name]
        self.symbols[name] = addr

    def add_variable(self, name, var):
        self.variables[name] = var

    def add_function(self, name, fn):
        self.functions[name] = fn

    def relocate(self, start, size):
        if self.size != size:
            print(self.size, 'vs.', size)
            return False
        diff = start - self.start
        self.symbols = {sym: addr + diff for sym, addr in self.symbols.items()}
        self.lines = {addr + diff: line for addr, line in self.lines.items()}
        self.start = start
        return True

    def cleanup(self, extra_lines):
        # common symbols have their source + line information,
        # but must be matched with actual definitions in DATA and BSS sections
        if False:
            for el in extra_lines:
                for s in self.symbols:
                    if s.name == el.name or s.name[1:] == el.name:
                        s.name = el.name
                        sl = SourceLine(s.address, el.src_file, el.line)
                        self.lines.append(sl)
        # self.lines = sorted(self.lines)

    def has_address(self, addr):
        return self.start <= addr and addr < self.end

    def find_addr_of_line(self, addr):
        if self.has_address(addr):
            symbols = [SourceLine(address=s.address) for s in self.symbols]
            lines = filter(lambda sl: sl.address <= addr, self.lines + symbols)
            return max(lines, key=lambda e: e.address)

    @property
    def symbol_table(self):
        return [Symbol(*s)
                for s in sorted(self.symbols.items(), key=lambda x: x[1])]

    def find_symbol(self, addr):
        if self.has_address(addr):
            symbols = [s for s in self.symbol_table if s.addr <= addr]
            if symbols:
                return symbols[-1]

    def find_symbol_addr(self, name):
        return self.symbols.get(name, None)

    @property
    def line_table(self):
        return sorted(self.lines.items())

    def find_line_addr(self, where):
        path, line = '', 0

        try:
            path, line = where.split(':')
            line = int(line)
        except ValueError:
            return None

        for addr, sl in self.line_table:
            if sl.path.endswith(path) and sl.line >= line:
                return addr

    def find_line(self, where):
        for addr, sl in self.line_table:
            if addr >= where:
                return sl
        return None

    def find_function(self, name):
        return self.functions.get(name, None)

    def find_variable(self, name):
        return self.variables.get(name, None)

    def dump_functions(self):
        print(' FUNCTIONS:')
        for name, fn in sorted(self.functions.items()):
            print('  {} <{}> in {} at {:08X}'.format(
                fn.attr[0], name, fn.line, self.find_symbol_addr(name)))
            print('   return type: {}'.format(
                fn.unit.get_type_name(fn.type)))
            for sc in fn.scopes:
                print('   {}:'.format(sc))
                for p in sc.values:
                    typnam = fn.unit.get_type_name(p.type)
                    if isinstance(p, ParamStk):
                        print('    {} {} (on stack, with offset {}) at {}'
                              .format(typnam, p.name, p.offset, p.line))
                    else:
                        regname = Registers.names[p.reg]
                        print('    {} {} (in register {}) at {}'
                              .format(typnam, p.name, regname, p.line))

    def dump_variables(self):
        print(' VARIABLES:')
        for name, var in sorted(self.variables.items()):
            print('  {} <{}> in {} at {:08X}'.format(
                var.attr[0], name, var.line, self.find_symbol_addr(name)))
            print('   type: {}'.format(
                var.unit.get_type_name(var.type)))

    def dump(self):
        print('{}:'.format(self))
        if False:
            print(' SYMBOLS:')
            for sym in self.symbol_table:
                print('  {:08X}: {}'.format(sym.addr, sym.name))
            print(' LINES:')
            for addr, line in sorted(self.lines.items()):
                print('  {:08X}: {}'.format(addr, line))
        if self.variables:
            self.dump_variables()
        if self.functions:
            self.dump_functions()

    def __str__(self):
        return '{:4} [{:08X} - {:08X}]'.format(
                self.hunk.type[5:], self.start, self.end)


class DebugInfo():
    def __init__(self):
        self.sections = []
        self.sources = {}
        self.units = {}

    def add_section(self, sec):
        self.sections.append(sec)

    @property
    def last_section(self):
        return self.sections[-1]

    def get_unit(self, filename):
        try:
            unit = self.units[filename]
        except KeyError:
            unit = TranslationUnit(filename)
            self.units[filename] = unit
        return unit

    def get_source(self, filename):
        try:
            source = self.sources[filename]
        except KeyError:
            source = SourceFile(filename)
            self.sources[filename] = source
        return source

    def cleanup(self, common_lines):
        for sec in self.sections:
            sec.cleanup(common_lines)

    def relocate(self, segments):
        if len(self.sections) != len(segments):
            return False
        return all(sec.relocate(seg.start, seg.size)
                   for sec, seg in zip(self.sections, segments))

    def dump(self):
        for sec in self.sections:
            sec.dump()
        print('units', self.units)
        for path, unit in self.units.items():
            unit.dump()

    def _find(self, func):
        for sec in self.sections:
            value = func(sec)
            if value is not None:
                return (sec, value)
        return None

    def find_line_addr(self, where):
        return self._find(lambda sec: sec.find_line_addr(where))

    def find_symbol_addr(self, name):
        return self._find(lambda sec: sec.find_symbol_addr(name))

    def find_line(self, addr):
        return self._find(lambda sec: sec.find_line(addr))

    def find_symbol(self, addr):
        return self._find(lambda sec: sec.find_symbol(addr))

    def find_function(self, name):
        return self._find(lambda sec: sec.find_function(name))

    def find_variable(self, name):
        return self._find(lambda sec: sec.find_variable(name))


def DebugInfoReader(executable):
    stab_to_section = {'GSYM': 'COMMON', 'STSYM': 'DATA', 'LCSYM': 'BSS'}

    common = Section(None)
    last = {'CODE': None, 'DATA': None, 'BSS': None, 'COMMON': common}
    start = 0
    size = 0

    di = DebugInfo()

    for h in ReadHunkFile(executable):

        if h.type in ['HUNK_CODE', 'HUNK_DATA', 'HUNK_BSS']:
            start += size
            size = h.size
            sec = Section(h, start, size)
            last[h.type[5:]] = sec
            di.add_section(sec)

        elif h.type is 'HUNK_SYMBOL':
            for s in h.symbols:
                addr, name = s.refs + start, s.name
                if name[0] == '_':
                    name = name[1:]
                di.last_section.add_symbol(name, addr)

        elif h.type is 'HUNK_DEBUG':
            stabs = h.data

            # h.dump()

            fn = None
            dirname = ''
            source = None
            t_sym = None
            scope = None
            stabinfo = None
            unit = None

            for st in stabs:
                # print(st)

                # N_SO: path and name of source file (selects object file)
                if st.type == 'SO':
                    if st.str and st.str[-1] == '/':
                        dirname = st.str
                    else:
                        source = di.get_source(dirname + st.str)
                        unit = di.get_unit(dirname + st.str)
                        stabinfo = StabInfoParser(unit)

                    fn = None
                    scope = None

                # N_SOL: used to switch source file (but not object file)
                elif st.type == 'SOL':
                    source = di.get_source(st.str)

                # N_DATA: data symbol
                # N_BSS: BSS symbol
                elif st.type in ['DATA', 'BSS']:
                    last[st.type].add_symbol(st.str, st.value)

                # N_GSYM: global symbol
                # N_STSYM: data segment file-scope variable
                # N_LCSYM: BSS segment file-scope variable
                elif st.type in ['GSYM', 'STSYM', 'LCSYM']:
                    if stabinfo.feed(st.str):
                        si = stabinfo.parse()
                        sl = source.line(st.desc)
                        var = Variable(unit, sl, si.attr, si.type)
                        sec = last[stab_to_section[st.type]]
                        sec.add_symbol(si.name, st.value)
                        sec.add_line(st.value, sl)
                        sec.add_variable(si.name, var)

                # N_LSYM: stack variable or type
                elif st.type in ['LSYM']:
                    if stabinfo.feed(st.str):
                        si = stabinfo.parse()

                # N_TEXT: file scope text symbol
                elif st.type in ['TEXT']:
                    last['CODE'].add_symbol(st.str, st.value)

                # N_SLINE: line number in text segment
                elif st.type in ['SLINE']:
                    # address, path, line, symbol
                    last['CODE'].add_line(st.value, source.line(st.desc))

                # N_FUN: function name or text segment variable
                elif st.type in ['FUN']:
                    sec = last['CODE']
                    si = stabinfo.parse(st.str)
                    sl = source.line(st.desc)
                    if fn:
                        fn.add_scope(scope.finish(st.value))
                    fn = Function(unit, sl, si.attr, si.type, [])
                    sec.add_line(st.value, sl)
                    sec.add_symbol(si.name, st.value)
                    sec.add_function(si.name, fn)
                    if not scope:
                        scope = ScopeStabParser(st.value)
                    scope.new(st.value)

                elif st.type in ['LBRAC']:
                    scope.enter(st.value)

                elif st.type in ['RBRAC']:
                    fn.add_scope(scope.leave(st.value))

                # N_RSYM: register variable
                elif st.type in ['RSYM']:
                    if stabinfo.feed(st.str):
                        si = stabinfo.parse()
                        sl = source.line(st.desc)
                        pr = ParamReg(si.name, sl, st.value, si.type)
                        scope.add(pr)

                # N_PSYM: parameter variable
                elif st.type in ['PSYM']:
                    if stabinfo.feed(st.str):
                        si = stabinfo.parse()
                        sl = source.line(st.desc)
                        ps = ParamStk(si.name, sl, st.value, si.type)
                        scope.add(ps)

                # N_SETA: Absolute set element
                # N_SETT: Text segment set element
                elif st.type in ['SETA', 'SETT']:
                    # TODO: how to parse linker sets ?
                    pass

                else:
                    raise ValueError('%s: not handled!', st.type)

    di.cleanup(common.lines)

    return di
