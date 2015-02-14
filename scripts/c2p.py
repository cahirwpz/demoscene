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

  def _index(self, i):
    return (self._width - 1) - i

  def _slice(self, s):
    return slice(self._index(s.start), self._index(s.stop - 1))

  def __getitem__(self, index):
    if type(index) == slice:
      s = self._slice(index)
      return Word(self._value[s])
    else:
      i = self._index(index)
      return self._value[i]

  def __setitem__(self, index, value):
    if type(index) == slice:
      s = self._slice(index)
      self._value[s] = value
    else:
      i = self._index(index)
      self._value[i] = value

  def __len__(self):
    return self._width

  def lsr(self, n):
    w = Word(len(self))
    w[(len(w) - 1) - n: 0] = self[len(w) - 1: n]
    return w

  def lsl(self, n):
    w = Word(len(self))
    w[(len(w) - 1):n] = self[(len(w) - 1 - n):0]
    return w

  def ror(self, n):
    w = Word(len(self))
    w[(len(w) - 1) - n: 0] = self[len(w) - 1: n]
    w[(len(w) - 1):(len(w) - n)] = self[n - 1:0]
    return w

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


def c2p_2_1_4():
  m0 = Word.Mask('33333333')
  m1 = Word.Mask('cccccccc')
  m2 = Word.Mask('55555555')
  m3 = Word.Mask('aaaaaaaa')

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
  m0 = Word.Mask('0f0f0f0f')
  m1 = Word.Mask('f0f0f0f0')
  m2 = Word.Mask('33333333')
  m3 = Word.Mask('cccccccc')
  m4 = Word.Mask('55555555')
  m5 = Word.Mask('aaaaaaaa')

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

def c2p_1_1_4_accurate():
  m0 = Word.Mask('00ff')
  m1 = Word.Mask('ff00')
  m2 = Word.Mask('0f0f')
  m3 = Word.Mask('f0f0')
  m4 = Word.Mask('3333')
  m5 = Word.Mask('cccc')
  m6 = Word.Mask('5555')
  m7 = Word.Mask('aaaa')

  print "=[ c2p 1x1 4bpl (accurate) ]=".center(48, '-')

  def MakeWord(chars, color):
    bits = []
    for c in chars:
      for i in range(4):
        bits.append(Bit.Var(c, i, color))
    return Word(bits)

  l0 = MakeWord("abcd", color="31m")
  l1 = MakeWord("efgh", color="32m")
  l2 = MakeWord("ijkl", color="33m")
  l3 = MakeWord("mnop", color="34m")

  printall("Data:", l0, l1, l2, l3)

  a0 = (l0 & m1) | (l2.lsr(8) & m0)
  a1 = (l1 & m1) | (l3.lsr(8) & m0)
  a2 = (l2 & m0) | (l0.lsl(8) & m1)
  a3 = (l3 & m0) | (l1.lsl(8) & m1)

  printall("Swap 8x4:", a0, a1, a2, a3)

  b0 = (a0 & m3) | (a1.lsr(4) & m2)
  b1 = (a1 & m2) | (a0.lsl(4) & m3)
  b2 = (a2 & m3) | (a3.lsr(4) & m2)
  b3 = (a3 & m2) | (a2.lsl(4) & m3)

  printall("Swap 4x2:", b0, b1, b2, b3)

  c0 = (b0 & m5) | (b2.lsr(2) & m4)
  c1 = (b1 & m5) | (b3.lsr(2) & m4)
  c2 = (b2 & m4) | (b0.lsl(2) & m5)
  c3 = (b3 & m4) | (b1.lsl(2) & m5)

  printall("Swap 2x2:", c0, c1, c2, c3)

  d0 = (c0 & m7) | (c1.lsr(1) & m6)
  d1 = (c1 & m6) | (c0.lsl(1) & m7)
  d2 = (c2 & m7) | (c3.lsr(1) & m6)
  d3 = (c3 & m6) | (c2.lsl(1) & m7)

  printall("Swap 1x1:", d0, d1, d2, d3)


def c2p_1_1_4_mangled():
  m0 = Word.Mask('00ff')
  m1 = Word.Mask('ff00')
  m2 = Word.Mask('0f0f')
  m3 = Word.Mask('f0f0')
  m4 = Word.Mask('3333')
  m5 = Word.Mask('cccc')

  print "=[ c2p 1x1 4bpl (mangled) ]=".center(48, '-')

  def MakeWord(c, color):
    return Word([Bit.Var(c[0], 0, color),
                 Bit.Var(c[1], 0, color),
                 Bit.Var(c[0], 1, color),
                 Bit.Var(c[1], 1, color),
                 Bit.Var(c[0], 2, color),
                 Bit.Var(c[1], 2, color),
                 Bit.Var(c[0], 3, color),
                 Bit.Var(c[1], 3, color),
                 Bit.Var(c[2], 0, color),
                 Bit.Var(c[3], 0, color),
                 Bit.Var(c[2], 1, color),
                 Bit.Var(c[3], 1, color),
                 Bit.Var(c[2], 2, color),
                 Bit.Var(c[3], 2, color),
                 Bit.Var(c[2], 3, color),
                 Bit.Var(c[3], 3, color)])

  l0 = MakeWord("abcd", color="31m")
  l1 = MakeWord("efgh", color="32m")
  l2 = MakeWord("ijkl", color="33m")
  l3 = MakeWord("mnop", color="34m")

  printall("Data:", l0, l1, l2, l3)

  a0 = (l0 & m1) | (l2.lsr(8) & m0)
  a1 = (l1 & m1) | (l3.lsr(8) & m0)
  a2 = (l2 & m0) | (l0.lsl(8) & m1)
  a3 = (l3 & m0) | (l1.lsl(8) & m1)

  printall("Swap 8x4:", a0, a1, a2, a3)

  b0 = (a0 & m3) | (a1.lsr(4) & m2)
  b1 = (a1 & m2) | (a0.lsl(4) & m3)
  b2 = (a2 & m3) | (a3.lsr(4) & m2)
  b3 = (a3 & m2) | (a2.lsl(4) & m3)

  printall("Swap 4x2:", b0, b1, b2, b3)

  c0 = (b0 & m5) | (b2.lsr(2) & m4)
  c1 = (b2 & m4) | (b0.lsl(2) & m5)
  c2 = (b1 & m5) | (b3.lsr(2) & m4)
  c3 = (b3 & m4) | (b1.lsl(2) & m5)

  printall("Swap 2x2:", c0, c1, c2, c3)


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

  a = ((l0 & m1) | (l1.lsr(4) & m0)) + ((l2 & m1) | (l3.lsr(4) & m0))
  b = ((l0.lsl(4) & m1) | (l1 & m0)) + ((l2.lsl(4) & m1) | (l3 & m0))

  printall("Swap 4x2:", a, b)

  x = (a & m2) | (a.lsr(1) & m3)
  y = (a & m3) | (a.lsl(1) & m2)
  z = (b & m2) | (b.lsr(1) & m3)
  w = (b & m3) | (b.lsl(1) & m2)

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


c2p_1_1_4_accurate()
#c2p_1_1_4_mangled()
#c2p_2_1_4_mangled()
#c2p_2_1_4_stingray()
