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

  w0 = Word.Data('a', color="31m")
  w1 = Word.Data('b', color="32m")
  w2 = Word.Data('c', color="33m")
  w3 = Word.Data('d', color="34m")

  printall("Step 1 (swap 2x4):", w0, w1, w2, w3)

  w4 = (w0 & m1) | (w2 & m0)
  w5 = (w1 & m1) | (w3 & m0)
  w6 = (w0 & m0) | (w2 & m1)
  w7 = (w1 & m0) | (w3 & m1)

  printall("Step 2 (rotate 2x2):", w4, w5, w6, w7)

  wa = (w4 & m3) | (w5 & m3).lsr(1)
  wb = (w4 & m2).lsl(1) | (w5 & m2)
  wc = (w6 & m3) | (w7 & m3).lsr(1)
  wd = (w6 & m2).lsl(1) | (w7 & m2)

  printall("Step 3 (rotate 4x4):", wa, wb, wc, wd)

  we = (wa & m1) | (wc & m1).lsr(2)
  wf = (wb & m1) | (wd & m1).lsr(2)
  wg = (wa & m0) | (wc & m0).lsl(2)
  wh = (wb & m0) | (wd & m0).lsl(2)

  printall("Result:", we, wg, wg, wh)


c2p_2_1_4()
