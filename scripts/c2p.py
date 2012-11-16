#!/usr/bin/env python

from itertools import *


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
      raise ValueError("One of arguments have to be a constant.")

  def __and__(self, bit):
    a = self._value
    b = bit._value

    if type(a) == int:
      return Bit(a and b, bit._color)
    elif type(b) == int:
      return Bit(b and a, self._color)
    else:
      raise ValueError("One of arguments have to be a constant.")

  def __str__(self):
    return "\033[%s%s%s" % (self._color, str(self._value).rjust(3), "\033[0m")


class Word(object):
  @classmethod
  def Mask(cls, value, width=32, color="0m"):
    return cls(Bit.Const((value >> ((width - 1) - i)) & 1, color) for i in range(width))

  @classmethod
  def Data(cls, char, width=32, color="0m"):
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
    w[(len(w) - 1):(len(w)-n)] = self[n-1:0]
    return w

  def __or__(self, word):
    if len(self) != len(word):
      raise ValueError("Operands length mismatch.")

    return Word(a | b for a, b in zip(self, word))

  def __and__(self, word):
    if len(self) != len(word):
      raise ValueError("Operands length mismatch.")

    return Word(a & b for a, b in zip(self, word))

  def __str__(self):
    return "".join(map(str, self))


def c2p_2_1_4():
  m0 = Word.Mask(0x33333333)
  m1 = Word.Mask(0xcccccccc)
  m2 = Word.Mask(0x55555555)
  m3 = Word.Mask(0xaaaaaaaa)

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
  m0 = Word.Mask(0x0f0f0f0f)
  m1 = Word.Mask(0xf0f0f0f0)
  m2 = Word.Mask(0x33333333)
  m3 = Word.Mask(0xcccccccc)
  m4 = Word.Mask(0x55555555)
  m5 = Word.Mask(0xaaaaaaaa)

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


c2p_2_1_4()
c2p_1_1_8()
