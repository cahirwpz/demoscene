#!/usr/bin/env python

from itertools import chain


def printall(*lines):
  for line in lines:
    print line


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

    if type(a) == int:
      return Bit(a or b, bit._color)
    elif type(b) == int:
      return Bit(b or a, self._color)
    else:
      return Bit('?', None)

  def __and__(self, bit):
    a = self._value
    b = bit._value

    if type(a) == int:
      return Bit(a and b, bit._color)
    elif type(b) == int:
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
    return "\033[%s%s%s" % (self._color, str(self._value).rjust(3), "\033[0m")


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
    if type(value_or_width) == int:
      self._value = [Bit.Const(0) for i in range(value_or_width)]
      self._width = value_or_width
    else:
      self._value = list(value_or_width)
      self._width = len(self._value)

  def __iter__(self):
    return iter(self._value)

  def __getitem__(self, index):
    if type(index) == slice:
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


class Channel(object):
  def __init__(self, data, start, modulo):
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


def MakeArray(MakeWord):
  return [MakeWord("abcd", color="31m"),
          MakeWord("efgh", color="32m"),
          MakeWord("ijkl", color="33m"),
          MakeWord("mnop", color="34m"),
          MakeWord("qrst", color="35m"),
          MakeWord("uvwx", color="36m"),
          MakeWord("ABCD", color="31;1m"),
          MakeWord("EFGH", color="32;1m"),
          MakeWord("IJKL", color="33;1m"),
          MakeWord("MNOP", color="34;1m"),
          MakeWord("QRST", color="35;1m"),
          MakeWord("UVWX", color="36;1m")]


def FreshArray(size, bits):
  return [Word(bits) for i in range(size)]


def FreshBitplanes(planes, size, bits):
  return [[Word(bits) for i in range(size)] for j in range(planes)]


def Blit(fn, height, width, A, B, C):
  for i in range(height):
    for j in range(width):
      C.put(fn(A.get(), B.get()))
    A.next()
    B.next()
    C.next()


def c2p_2_1_4():
  m0 = Word.Mask('3333')
  m1 = Word.Mask('cccc')
  m2 = Word.Mask('5555')
  m3 = Word.Mask('aaaa')

  print "c2p 2x1 4-bitplanes".center(96, '-')

  a0 = Word.Data('a', color="31m")
  a1 = Word.Data('b', color="32m")
  a2 = Word.Data('c', color="33m")
  a3 = Word.Data('d', color="34m")

  printall("Data:", a0, a1, a2, a3)

  b0 = (a0 & m1) | (a2.lsr(2) & m0)
  b1 = (a1 & m1) | (a3.lsr(2) & m0)
  b2 = (a0.lsl(2) & m1) | (a2 & m0)
  b3 = (a1.lsl(2) & m1) | (a3 & m0)

  printall("Swap 2x2:", b0, b1, b2, b3)

  c0 = (b0 & m3) | (b1.lsr(1) & m2)
  c1 = (b0.lsl(1) & m3) | (b1 & m2)
  c2 = (b2 & m3) | (b3.lsr(1) & m2)
  c3 = (b2.lsl(1) & m3) | (b3 & m2)

  printall("Swap 1x1:", c0, c1, c2, c3)


def c2p_1_1_8():
  m0 = Word.Mask('0f0f')
  m1 = Word.Mask('f0f0')
  m2 = Word.Mask('3333')
  m3 = Word.Mask('cccc')
  m4 = Word.Mask('5555')
  m5 = Word.Mask('aaaa')

  print "c2p 1x1 8-bitplanes".center(96, '-')

  a0 = Word.Data('a', color="31m")
  a1 = Word.Data('b', color="32m")
  a2 = Word.Data('c', color="33m")
  a3 = Word.Data('d', color="34m")
  a4 = Word.Data('e', color="35m")
  a5 = Word.Data('f', color="36m")
  a6 = Word.Data('g', color="37m")
  a7 = Word.Data('h', color="38m")

  printall("Data:", a0, a1, a2, a3, a4, a5, a6, a7)

  b0 = (a0 & m1) | (a4.lsr(4) & m0)
  b1 = (a1 & m1) | (a5.lsr(4) & m0)
  b2 = (a2 & m1) | (a6.lsr(4) & m0)
  b3 = (a3 & m1) | (a7.lsr(4) & m0)
  b4 = (a0.lsl(4) & m1) | (a4 & m0)
  b5 = (a1.lsl(4) & m1) | (a5 & m0)
  b6 = (a2.lsl(4) & m1) | (a6 & m0)
  b7 = (a3.lsl(4) & m1) | (a7 & m0)

  printall("Swap 4x4:", b0, b1, b2, b3, b4, b5, b6, b7)

  c0 = (b0 & m3) | (b2.lsr(2) & m2)
  c1 = (b1 & m3) | (b3.lsr(2) & m2)
  c2 = (b0.lsl(2) & m3) | (b2 & m2)
  c3 = (b1.lsl(2) & m3) | (b3 & m2)
  c4 = (b4 & m3) | (b6.lsr(2) & m2)
  c5 = (b5 & m3) | (b7.lsr(2) & m2)
  c6 = (b4.lsl(2) & m3) | (b6 & m2)
  c7 = (b5.lsl(2) & m3) | (b7 & m2)

  printall("Swap 2x2:", c0, c1, c2, c3, c4, c5, c6, c7)

  d0 = (c0 & m5) | (c1.lsr(1) & m4)
  d1 = (c0.lsl(1) & m5) | (c1 & m4)
  d2 = (c2 & m5) | (c3.lsr(1) & m4)
  d3 = (c2.lsl(1) & m5) | (c3 & m4)
  d4 = (c4 & m5) | (c5.lsr(1) & m4)
  d5 = (c4.lsl(1) & m5) | (c5 & m4)
  d6 = (c6 & m5) | (c7.lsr(1) & m4)
  d7 = (c6.lsl(1) & m5) | (c7 & m4)

  printall("Swap 1x1:", d0, d1, d2, d3, d4, d5, d6, d7)


def c2p_1_1_4_blitter(bitplane_output=True):
  m0 = Word.Mask('00ff')
  m1 = Word.Mask('0f0f')
  m2 = Word.Mask('3333')
  m3 = Word.Mask('5555')

  print "=[ c2p 1x1 4bpl (blitter) ]=".center(48, '-')

  def MakeWord(chars, color):
    bits = []
    for c in chars:
      for i in range(4):
        bits.append(Bit.Var(c, i, color))
    return Word(bits)

  A = MakeArray(MakeWord)
  N = len(A)
  printall("Data:", *A)

  B = FreshArray(N, 16)
  Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
       N / 4, 2, Channel(A, 2, 2), Channel(A, 0, 2), Channel(B, 0, 2))
  Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
       N / 4, 2, Channel(A, 0, 2), Channel(A, 2, 2), Channel(B, 2, 2))
  printall("Swap 8x4:", *B)

  C = FreshArray(N, 16)
  Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
       N / 2, 1, Channel(B, 1, 1), Channel(B, 0, 1), Channel(C, 0, 1))
  Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 1, 1), Channel(C, 1, 1))
  printall("Swap 4x2:", *C)

  D = FreshArray(N, 16)
  Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
       N / 4, 2, Channel(C, 2, 2), Channel(C, 0, 2), Channel(D, 0, 2))
  Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
       N / 4, 2, Channel(C, 0, 2), Channel(C, 2, 2), Channel(D, 2, 2))
  printall("Swap 2x2:", *D)

  if bitplane_output:
    E = FreshBitplanes(4, N / 4, 16)
    Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3),
         N / 4, 1, Channel(D, 1, 3), Channel(D, 0, 3), Channel(E[0], 0, 0))
    Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3),
         N / 4, 1, Channel(D, 3, 3), Channel(D, 2, 3), Channel(E[2], 0, 0))
    Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3),
         N / 4, 1, Channel(D, 0, 3), Channel(D, 1, 3), Channel(E[1], 0, 0))
    Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3),
         N / 4, 1, Channel(D, 2, 3), Channel(D, 3, 3), Channel(E[3], 0, 0))
    print("Bitplanes:")
    printall("[0]:", *E[0])
    printall("[1]:", *E[1])
    printall("[2]:", *E[2])
    printall("[3]:", *E[3])
  else:
    E = FreshArray(N, 16)
    Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3),
         N / 2, 1, Channel(D, 1, 1), Channel(D, 0, 1), Channel(E, 0, 1))
    Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3),
         N / 2, 1, Channel(D, 0, 1), Channel(D, 1, 1), Channel(E, 1, 1))
    printall("Swap 1x1:", *E)


def c2p_1_1_4_blitter_mangled(bitplane_output=True):
  m0 = Word.Mask('00ff')
  m1 = Word.Mask('0f0f')
  m2 = Word.Mask('3333')

  print "=[ c2p 1x1 4bpl (blitter mangled) ]=".center(48, '-')

  def MakeWord(c, color):
    return Word([Bit.Var(c[0], 0, color),
                 Bit.Var(c[1], 0, color),
                 Bit.Var(c[0], 2, color),
                 Bit.Var(c[1], 2, color),
                 Bit.Var(c[0], 1, color),
                 Bit.Var(c[1], 1, color),
                 Bit.Var(c[0], 3, color),
                 Bit.Var(c[1], 3, color),
                 Bit.Var(c[2], 0, color),
                 Bit.Var(c[3], 0, color),
                 Bit.Var(c[2], 2, color),
                 Bit.Var(c[3], 2, color),
                 Bit.Var(c[2], 1, color),
                 Bit.Var(c[3], 1, color),
                 Bit.Var(c[2], 3, color),
                 Bit.Var(c[3], 3, color)])

  A = MakeArray(MakeWord)
  N = len(A)
  printall("Data:", *A)

  B = FreshArray(N, 16)
  Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
       N / 4, 2, Channel(A, 2, 2), Channel(A, 0, 2), Channel(B, 0, 2))
  Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
       N / 4, 2, Channel(A, 0, 2), Channel(A, 2, 2), Channel(B, 2, 2))
  printall("Swap 8x4:", *B)

  C = FreshArray(N, 16)
  Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
       N / 2, 1, Channel(B, 1, 1), Channel(B, 0, 1), Channel(C, 0, 1))
  Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 1, 1), Channel(C, 1, 1))
  printall("Swap 4x2:", *C)

  if bitplane_output:
    D = FreshBitplanes(4, N / 4, 16)
    Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
         N / 4, 1, Channel(C, 2, 3), Channel(C, 0, 3), Channel(D[0], 0, 0))
    Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
         N / 4, 1, Channel(C, 3, 3), Channel(C, 1, 3), Channel(D[1], 0, 0))
    Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
         N / 4, 1, Channel(C, 0, 3), Channel(C, 2, 3), Channel(D[2], 0, 0))
    Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
         N / 4, 1, Channel(C, 1, 3), Channel(C, 3, 3), Channel(D[3], 0, 0))
    print("Bitplanes:")
    printall("[0]:", *D[0])
    printall("[1]:", *D[1])
    printall("[2]:", *D[2])
    printall("[3]:", *D[3])
  else:
    D = FreshArray(N, 16)
    Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
         N / 4, 2, Channel(C, 2, 2), Channel(C, 0, 2), Channel(D, 0, 2))
    Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
         N / 4, 2, Channel(C, 0, 2), Channel(C, 2, 2), Channel(D, 2, 2))
    printall("Swap 2x2:", *D)


def c2p_2_1_4_mangled():
  m0 = Word.Mask('0f0f')
  m1 = Word.Mask('f0f0')
  m2 = Word.Mask('aaaaaaaa')
  m3 = Word.Mask('55555555')

  print "=[ c2p 2x1 4bpl (mangled) ]=".center(48, '-')

  def MakeWord(c, color):
    return Word([Bit.Var(c[0], 0, color),
                 Bit.Var(c[0], 1, color),
                 Bit.Var(c[1], 0, color),
                 Bit.Var(c[1], 1, color),
                 Bit.Var(c[0], 2, color),
                 Bit.Var(c[0], 3, color),
                 Bit.Var(c[1], 2, color),
                 Bit.Var(c[1], 3, color),
                 Bit.Var(c[2], 0, color),
                 Bit.Var(c[2], 1, color),
                 Bit.Var(c[3], 0, color),
                 Bit.Var(c[3], 1, color),
                 Bit.Var(c[2], 2, color),
                 Bit.Var(c[2], 3, color),
                 Bit.Var(c[3], 2, color),
                 Bit.Var(c[3], 3, color)])

  l0 = MakeWord("abef", color="31m")
  l1 = MakeWord("cdgh", color="32m")
  l2 = MakeWord("ijmn", color="33m")
  l3 = MakeWord("klop", color="34m")

  printall("Data:", l0, l1, l2, l3)

  a = ((l0 & m1) | ((l1 >> 4) & m0)) + ((l2 & m1) | ((l3 >> 4) & m0))
  b = (((l0 << 4) & m1) | (l1 & m0)) + (((l2 << 4) & m1) | (l3 & m0))

  printall("Swap 4x2:", a, b)

  x = (a & m2) | ((a >> 1) & m3)
  y = (a & m3) | ((a >> 1) & m2)
  z = (b & m2) | ((b >> 1) & m3)
  w = (b & m3) | ((b >> 1) & m2)

  printall("Bitplanes:", x, y, z, w)


def c2p_2_1_4_stingray():
  m0 = Word.Mask('f0f0')
  m1 = Word.Mask('0f0f')
  m2 = Word.Mask('aaaa')
  m3 = Word.Mask('5555')

  print "=[ c2p 2x1 4bpl (stingray) ]=".center(48, '-')

  def MakeWord(a, b, e, f, c):
    return Word([Bit.Var(a, 3, c),
                 Bit.Var(a, 2, c),
                 Bit.Var(b, 3, c),
                 Bit.Var(b, 2, c),
                 Bit.Var(a, 1, c),
                 Bit.Var(a, 0, c),
                 Bit.Var(b, 1, c),
                 Bit.Var(b, 0, c),
                 Bit.Var(e, 3, c),
                 Bit.Var(e, 2, c),
                 Bit.Var(f, 3, c),
                 Bit.Var(f, 2, c),
                 Bit.Var(e, 1, c),
                 Bit.Var(e, 0, c),
                 Bit.Var(f, 1, c),
                 Bit.Var(f, 0, c)])

  i0 = MakeWord('a', 'b', 'e', 'f', "31m")
  i1 = MakeWord('c', 'd', 'g', 'h', "32m")
  i2 = MakeWord('A', 'B', 'E', 'F', "33m")
  i3 = MakeWord('C', 'D', 'G', 'H', "34m")

  printall("Data:", i0, i1, i2, i3)

  a0 = (i0.lsl(4) & m0) | (i1 & m1)
  a2 = (i2.lsl(4) & m0) | (i3 & m1)
  a1 = (i0 & m0) | (i1.lsr(4) & m1)
  a3 = (i2 & m0) | (i3.lsr(4) & m1)

  printall("Swap 4x2:", a0, a1, a2, a3)

  b0 = a0 & m3
  b2 = a1 & m3
  b4 = a2 & m3
  b6 = a3 & m3

  b1 = a0.lsr(1) & m3
  b3 = a1.lsr(1) & m3
  b5 = a2.lsr(1) & m3
  b7 = a3.lsr(1) & m3

  printall("Bitplanes:", b0, b1, b2, b3, b4, b5, b6, b7)

  b0 = (a0 & m3) | (a0.lsl(1) & m2)
  b2 = (a1 & m3) | (a1.lsl(1) & m2)
  b4 = (a2 & m3) | (a2.lsl(1) & m2)
  b6 = (a3 & m3) | (a3.lsl(1) & m2)

  b1 = (a0.lsr(1) & m3) | (a0 & m2)
  b3 = (a1.lsr(1) & m3) | (a1 & m2)
  b5 = (a2.lsr(1) & m3) | (a2 & m2)
  b7 = (a3.lsr(1) & m3) | (a3 & m2)

  printall("Bitplanes:", b0, b1, b2, b3, b4, b5, b6, b7)


c2p_1_1_4_blitter()
#c2p_1_1_4_blitter_mangled()
#c2p_2_1_4_mangled()
#c2p_2_1_4_stingray()
