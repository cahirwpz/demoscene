#!/usr/bin/env python -B

from common import Bit, Word, Array, Blit, Channel


def c2p(bitplane_output=True):
  print "=[ c2p 2x1 4bpl (blitter + mangled) ]=".center(48, '-')

  # premangled pixels:
  # 1) [- - - - a b c d] => [a b - - c d - -]
  # 2) [- - - - e f g h] => [- - e f - - g h]
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

  # premangled pixel buffer: [AB CD EF GH] => [AB EF CD GH]
  def ArrayMake(fn):
    return [fn("abef", color="31m"),
            fn("cdgh", color="32m"),
            fn("ijmn", color="33m"),
            fn("klop", color="34m"),
            fn("qruv", color="35m"),
            fn("stwx", color="36m"),
            fn("ABEF", color="31;1m"),
            fn("CDGH", color="32;1m"),
            fn("IJMN", color="33;1m"),
            fn("KLOP", color="34;1m"),
            fn("QRUV", color="35;1m"),
            fn("STWX", color="36;1m")]

  A = ArrayMake(MakeWord)
  N = len(A)
  Array.Print("Data:", *A)

  m0 = Word.Mask('f0f0')
  m1 = Word.Mask('aaaa')

  B = Array.Zero(N, 16)
  Blit(lambda a, b: (a & m0) | ((b >> 4) & ~m0),
       N / 2, 1, Channel(A, 0, 1), Channel(A, 1, 1), Channel(B, 0, 1))
  Blit(lambda a, b: ((a << 4) & m0) | (b & ~m0),
       N / 2, 1, Channel(A, 0, 1), Channel(A, 1, 1), Channel(B, 1, 1))
  Array.Print("Swap 4x2:", *B)

  C = [Array.Zero(N / 2, 16) for i in range(4)]
  Blit(lambda a, b: (a & m1) | ((b >> 1) & ~m1),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 0, 1), Channel(C[0], 0, 0))
  Blit(lambda a, b: (a & m1) | ((b >> 1) & ~m1),
       N / 2, 1, Channel(B, 1, 1), Channel(B, 1, 1), Channel(C[2], 0, 0))
  Blit(lambda a, b: ((a << 1) & m1) | (b & ~m1),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 0, 1), Channel(C[1], 0, 0))
  Blit(lambda a, b: ((a << 1) & m1) | (b & ~m1),
       N / 2, 1, Channel(B, 1, 1), Channel(B, 1, 1), Channel(C[3], 0, 0))
  print("Bitplanes:")
  Array.Print("[0]:", *C[0])
  Array.Print("[1]:", *C[1])
  Array.Print("[2]:", *C[2])
  Array.Print("[3]:", *C[3])


c2p()
