#!/usr/bin/env python -B

from common import Bit, Word, Array, Blit, Channel


def c2p(bitplane_output=True):
  m0 = Word.Mask('00ff')
  m1 = Word.Mask('0f0f')
  m2 = Word.Mask('5555')

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

  A = Array.Make(MakeWord)
  N = len(A)
  Array.Print("Data:", *A)

  B = Array.Zero(N, 16)
  Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
       N / 2, 1, Channel(A, 1, 1), Channel(A, 0, 1), Channel(B, 0, 1))
  Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
       N / 2, 1, Channel(A, 0, 1), Channel(A, 1, 1), Channel(B, 1, 1))
  Array.Print("Swap 8x2:", *B)

  C = Array.Zero(N, 16)
  Blit(lambda a, b: ((a & ~m1) | ((b >> 4) & m1)),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 1, 1), Channel(C, 0, 1))
  Blit(lambda a, b: (((a << 4) & ~m1) | (b & m1)),
       N / 2, 1, Channel(B, 0, 1), Channel(B, 1, 1), Channel(C, 1, 1))
  Array.Print("Swap 4x2:", *C)

  if bitplane_output:
    D = [Array.Zero(N / 2, 16) for i in range(4)]
    Blit(lambda a, b: (((a >> 1) & m2) | (b & ~m2)),
         N / 2, 1, Channel(C, 0, 1), Channel(C, 0, 1), Channel(D[0], 0, 0))
    Blit(lambda a, b: (((a >> 1) & m2) | (b & ~m2)),
         N / 2, 1, Channel(C, 1, 1), Channel(C, 1, 1), Channel(D[2], 0, 0))
    Blit(lambda a, b: (((a << 1) & ~m2) | (b & m2)),
         N / 2, 1, Channel(C, 0, 1), Channel(C, 0, 1), Channel(D[1], 0, 0))
    Blit(lambda a, b: (((a << 1) & ~m2) | (b & m2)),
         N / 2, 1, Channel(C, 1, 1), Channel(C, 1, 1), Channel(D[3], 0, 0))
    print("Bitplanes:")
    Array.Print("[0]:", *D[0])
    Array.Print("[1]:", *D[1])
    Array.Print("[2]:", *D[2])
    Array.Print("[3]:", *D[3])
  else:
    D = Array.Zero(N * 2, 16)
    Blit(lambda a, b: (((a >> 1) & m2) | (b & ~m2)),
         N, 1, Channel(C, 0, 0), Channel(C, 0, 0), Channel(D, 0, 1))
    Blit(lambda a, b: (((a << 1) & ~m2) | (b & m2)),
         N, 1, Channel(C, 0, 0), Channel(C, 0, 0), Channel(D, 1, 1))
    Array.Print("Expand 2x1:", *D)


c2p()
