import struct
import re

from collections import namedtuple, Sequence


class StringTable(Sequence):
    def __init__(self):
        self._map = {}
        self._table = []

    def __getitem__(self, index):
        return self._table[index]

    def __len__(self):
        return len(self._table)

    def __iter__(self):
        return iter(self._table)

    def __contains__(self, item):
        return item in self._table

    def addString(self, offset, text):
        self._map[offset] = len(self._table)
        self._table.append(text)

    @classmethod
    def decode(cls, data):
        strings = cls()
        s = 0
        while True:
            e = data.find(b'\0', s)
            strings.addString(s, data[s:e].decode('ascii'))
            if e == -1:
                break
            s = e + 1
        return strings

    def stringAtOffset(self, offset):
        index = self._map.get(offset)
        if index is None:
            return ''
        return self._table[index]


class Stab(namedtuple('Stab',
                      ('str', 'type', 'ext', 'other', 'desc', 'value'))):
    # http://sourceware.org/gdb/current/onlinedocs/stabs/Stab-Types.html
    type_list = [
        ('UNDF', 0x00), ('EXT', 0x01), ('ABS', 0x02), ('TEXT', 0x04),
        ('DATA', 0x06), ('BSS', 0x08), ('INDR', 0x0a), ('SIZE', 0x0c),
        ('COMM', 0x12), ('SETA', 0x14), ('SETT', 0x16), ('SETD', 0x18),
        ('SETB', 0x1a), ('SETV', 0x1c), ('WARNING', 0x1e), ('FN', 0x1f),
        ('GSYM', 0x20), ('FNAME', 0x22), ('FUN', 0x24), ('STSYM', 0x26),
        ('LCSYM', 0x28), ('MAIN', 0x2a), ('ROSYM', 0x2c), ('PC', 0x30),
        ('NSYMS', 0x32), ('NOMAP', 0x34), ('MAC_DEFINE', 0x36), ('OBJ', 0x38),
        ('MAC_UNDEF', 0x3a), ('OPT', 0x3c), ('RSYM', 0x40), ('SLINE', 0x44),
        ('DSLINE', 0x46), ('BSLINE', 0x48), ('FLINE', 0x4c), ('EHDECL', 0x50),
        ('CATCH', 0x54), ('SSYM', 0x60), ('ENDM', 0x62), ('SO', 0x64),
        ('LSYM', 0x80), ('BINCL', 0x82), ('SOL', 0x84), ('PSYM', 0xa0),
        ('EINCL', 0xa2), ('ENTRY', 0xa4), ('LBRAC', 0xc0), ('EXCL', 0xc2),
        ('SCOPE', 0xc4), ('RBRAC', 0xe0), ('BCOMM', 0xe2), ('ECOMM', 0xe4),
        ('ECOML', 0xe8), ('WITH', 0xea), ('NBTEXT', 0xf0), ('NBDATA', 0xf2),
        ('NBBSS', 0xf4), ('NBSTS', 0xf6), ('NBLCS', 0xf8)]
    inv_type_map = dict((t, n) for n, t in type_list)

    @classmethod
    def decode(cls, data, strtab):
        _stroff, _bintype, _other, _desc, _value = \
                struct.unpack('>iBbhI', data)
        _str = strtab.stringAtOffset(_stroff)
        _ext = _bintype & 1
        _type = cls.inv_type_map.get(_bintype & ~1, 'DEBUG')
        return cls(_str, _type, _ext, _other, _desc, _value)

    def as_string(self):
        visibility = ['l', 'g'][self.ext]
        return '{3:08x} {5} {0:<5} {2:04x} {1:02x} {4}'.format(
            self.type, self.other, self.desc, self.value, self.str, visibility)


# https://sourceware.org/gdb/onlinedocs/stabs/
Types = namedtuple('Types', 'decls')
Entry = namedtuple('Entry', 'name value')
Field = namedtuple('Field', 'name type offset size')
StructType = namedtuple('StructType', 'size fields')
UnionType = namedtuple('UnionType', 'size fields')
EnumType = namedtuple('EnumType', 'values')
SizeOf = namedtuple('SizeOf', 'size type')
ArrayType = namedtuple('ArrayType', 'range type')
Function = namedtuple('Function', 'name attr type')
FunctionType = namedtuple('FunctionType', 'type')
Pointer = namedtuple('Pointer', 'type')
Parameter = namedtuple('Parameter', 'name attr type')
Register = namedtuple('Register', 'name type')
Variable = namedtuple('Variable', 'name attr type')
ForwardDecl = namedtuple('ForwardDecl', 'type name')
Info = namedtuple('Info', 'name info')
TypeDecl = namedtuple('TypeDecl', 'name type')


class Subrange(namedtuple('Subrange', 'itype lo hi')):
    @property
    def count(self):
        return self.hi - self.lo + 1


class StabInfoParser():
    def __init__(self, source):
        self._data = ''
        self._pos = 0
        self._source = source

    def read(self):
        try:
            char = self._data[self._pos]
            self._pos += 1
            return char
        except IndexError:
            return None

    def unread(self):
        self._pos -= 1

    def expect(self, char):
        try:
            last = self._data[self._pos]
        except IndexError:
            last = None
        if last != char:
            raise ValueError('Expected "%s" got "%s" in "%s"' %
                             (char, self.rest()[0], self._data))
        self._pos += 1

    def consume(self, regex):
        m = regex.match(self._data, self._pos)
        if not m:
            raise ValueError('Expected "%s" got "%s" in "%s"' %
                             (regex.pattern, self.rest(), self._data))
        self._pos = m.end()
        return m.group(0)

    def peek(self, char):
        try:
            last = self._data[self._pos]
        except IndexError:
            last = None
        if last != char:
            return False
        self._pos += 1
        return True

    def rest(self):
        return self._data[self._pos:]

    TLabel = re.compile('[A-Za-z0-9_ ]+')
    TNumber = re.compile('-?[0-9]+')

    def doLabel(self):
        return self.consume(self.TLabel)

    def doNumber(self):
        number = self.consume(self.TNumber)
        if number[0] == '0':
            return int(number, 8)
        return int(number)

    def doField(self):
        name, _ = self.doLabel(), self.expect(':')
        typ, _ = self.doTypeDecl(), self.expect(',')
        offset, _ = self.doNumber(), self.expect(',')
        size, _ = self.doNumber(), self.expect(';')
        return Field(name, typ, offset, size)

    def doType(self):
        last = self.read()

        if last == 'a':
            arange = self.doType()
            eltype = self.doTypeDecl()
            return ArrayType(arange, eltype)

        if last == 'r':
            _of, _ = self.doNumber(), self.expect(';')
            _lo, _ = self.doNumber(), self.expect(';')
            _hi, _ = self.doNumber(), self.expect(';')
            if _lo > 0 and _hi > 0:
                _lo = -_lo
            return Subrange(_of, _lo, _hi)

        if last == 's':
            size = self.doNumber()
            fields = []
            while not self.peek(';'):
                fields.append(self.doField())
            return StructType(size, fields)

        if last == 'u':
            size = self.doNumber()
            fields = []
            while not self.peek(';'):
                fields.append(self.doField())
            return UnionType(size, fields)

        if last == 'f' or last == 'F':
            return FunctionType(self.doNumber())

        if last == '*':
            return Pointer(self.doType())

        if last == 'x':
            if self.peek('s'):
                typ = 'struct'
            elif self.peek('u'):
                typ = 'union'
            else:
                raise ValueError(self.rest())
            name, _ = self.doLabel(), self.expect(':')
            return ForwardDecl(typ, name)

        if last == '@':
            if self.peek('s'):
                kind = 'struct'
            else:
                raise ValueError(self.rest())
            size, _, typ = self.doNumber(), self.expect(';'), self.doType()
            return SizeOf(size, typ)

        if last == 'e':
            entries = []
            while not self.peek(';'):
                name, _ = self.doLabel(), self.expect(':')
                value, _ = self.doNumber(), self.expect(',')
                entries.append(Entry(name, value))
            return EnumType(entries)

        self.unread()

        return self.doNumber()

    def doTypeDecl(self, name=None):
        ref = self.doType()
        if self.peek('='):
            if name:
                typedecl = TypeDecl(name, self.doTypeDecl())
            else:
                typedecl = self.doTypeDecl()
            if isinstance(ref, int):
                self._source.add_type(ref, typedecl)
            elif isinstance(ref, Pointer):
                self._source.add_type(ref.type, typedecl)
            else:
                raise ValueError(type(ref), self.rest())
            return typedecl
        return ref

    def doInfo(self):
        name, _ = self.doLabel(), self.expect(':')

        last = self.read()

        if last == 't' or last == 'T':
            return self.doTypeDecl(name)
        if last == 'G':
            return Variable(name, ['global'], self.doTypeDecl())
        if last == 'S':
            return Variable(name, ['local', 'file'], self.doTypeDecl())
        if last == 'V':
            return Variable(name, ['local', 'scope'], self.doTypeDecl())
        if last == 'f':
            return Function(name, ['local'], self.doTypeDecl())
        if last == 'F':
            return Function(name, ['global'], self.doTypeDecl())
        if last == 'r':
            return Register(name, self.doTypeDecl())
        if last == 'p':
            return Parameter(name, ['stack'], self.doTypeDecl())
        if last == 'P':
            return Parameter(name, ['register'], self.doTypeDecl())

        self.unread()

        return self.doTypeDecl(name)

    def parse(self, s=None):
        if s is not None:
            self._data = s
            self._pos = 0
        si = self.doInfo()
        if self._pos < len(self._data):
            raise ValueError(self.rest())
        self._data = ''
        self._pos = 0
        return si

    def feed(self, s):
        if s[-1] == '\\':
            self._data += s[:-1]
            return False
        self._data += s
        return True
