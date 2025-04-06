from itertools import chain


class Bit(object):

    @classmethod
    def Const(cls, value, color=None):
        return cls(value, color)

    @classmethod
    def Var(cls, name, number, color=None):
        if number < 10:
            number = str(number)
        else:
            number = chr((number - 10) + ord('A'))
        return cls("%s%c" % (name, number), color)

    def __init__(self, value, color):
        self._value = value
        self._color = color or "\033[0m"

    def __or__(self, bit):
        a = self._value
        b = bit._value

        if isinstance(a, int):
            return Bit(a or b, bit._color)
        elif isinstance(b, int):
            return Bit(b or a, self._color)
        else:
            return Bit('?', None)

    def __and__(self, bit):
        a = self._value
        b = bit._value

        if isinstance(a, int):
            return Bit(a and b, bit._color)
        elif isinstance(b, int):
            return Bit(b and a, self._color)
        else:
            return Bit('?', None)

    def __invert__(self):
        a = self._value
        if a == 1:
            return Bit(0, self._color)
        elif a == 0:
            return Bit(1, self._color)
        else:
            return Bit('?', None)

    def __str__(self):
        return "\033[%s%s\033[0m" % (self._color, str(self._value).rjust(3))


class Word(object):

    @classmethod
    def Mask(cls, value, color="0m"):
        width = len(value) * 4
        value = int(value, base=16)
        return cls(Bit.Const((value >> ((width - 1) - i)) & 1, color)
                   for i in range(width))

    @classmethod
    def Data(cls, char, width=16, color="0m"):
        return cls(Bit.Var(char, i, color) for i in range(width))

    def __init__(self, value_or_width):
        if isinstance(value_or_width, int):
            self._value = [Bit(0, "0m") for i in range(value_or_width)]
            self._width = value_or_width
        else:
            self._value = list(value_or_width)
            self._width = len(self._value)

    def __iter__(self):
        return iter(self._value)

    def __getitem__(self, index):
        if isinstance(index, slice):
            return Word(self._value[index])
        else:
            return self._value[index]

    def __setitem__(self, index, value):
        self._value[index] = value

    def __len__(self):
        return self._width

    def __rshift__(self, s):
        n = len(self)
        return Word(s) + self[:n - s]

    def __lshift__(self, s):
        return self[s:] + Word(s)

    def __invert__(self):
        return Word(~a for a in self)

    def ror(self, s):
        n = len(self)
        return self[n - s:] + self[:n - s]

    def rol(self, s):
        return self[s:] + self[:s]

    def __or__(self, word):
        if len(self) != len(word):
            raise ValueError("Operands length mismatch.")

        return Word(a | b for a, b in zip(self, word))

    def __and__(self, word):
        if len(self) != len(word):
            raise ValueError("Operands length mismatch.")

        return Word(a & b for a, b in zip(self, word))

    def __add__(self, word):
        return Word(list(chain(self, word)))

    def __str__(self):
        return "".join(map(str, self))


class Array(object):

    @staticmethod
    def Make(fn):
        return [fn("abcd", color="31m"),
                fn("efgh", color="32m"),
                fn("ijkl", color="33m"),
                fn("mnop", color="34m"),
                fn("qrst", color="35m"),
                fn("uvwx", color="36m"),
                fn("ABCD", color="31;1m"),
                fn("EFGH", color="32;1m"),
                fn("IJKL", color="33;1m"),
                fn("MNOP", color="34;1m"),
                fn("QRST", color="35;1m"),
                fn("UVWX", color="36;1m"),
                fn("abcd", color="31;7m"),
                fn("efgh", color="32;7m"),
                fn("ijkl", color="33;7m"),
                fn("mnop", color="34;7m"),
                fn("qrst", color="35;7m"),
                fn("uvwx", color="36;7m"),
                fn("ABCD", color="31;1;7m"),
                fn("EFGH", color="32;1;7m"),
                fn("IJKL", color="33;1;7m"),
                fn("MNOP", color="34;1;7m"),
                fn("QRST", color="35;1;7m"),
                fn("UVWX", color="36;1;7m")]

    @staticmethod
    def Zero(size, bits):
        return [Word(bits) for i in range(size)]

    @staticmethod
    def Print(*lines):
        for line in lines:
            print(line)


class Channel(object):

    def __init__(self, data, start=0, modulo=0):
        self.pos = start
        self.data = data
        self.modulo = modulo

    def get(self):
        n = self.data[self.pos]
        self.pos += 1
        return n

    def put(self, n):
        self.data[self.pos] = n
        self.pos += 1

    def next(self):
        self.pos += self.modulo


def Blit(fn, height, width, A, B, C):
    for i in range(height):
        for j in range(width):
            C.put(fn(A.get(), B.get()))
        A.next()
        B.next()
        C.next()
