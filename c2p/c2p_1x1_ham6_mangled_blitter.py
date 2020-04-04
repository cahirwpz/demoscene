#!/usr/bin/env python -B

from common import Bit, Word, Channel, Blit, Array


def c2p():
    m0 = Word.Mask('00ff')
    m1 = Word.Mask('0f0f')

    print "=[ c2p 1x1 ham6 (blitter + mangled) ]=".center(48, '-')

    def MakeWord(c, color):
        # mangle: [ 0  0  0  0 r0 r1 r2 r3 g0 g1 g2 g3 b0 b1 b2 b3] =>
        #         [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3]
        return Word([Bit.Var(c[0], 0, color),
                     Bit.Var(c[1], 0, color),
                     Bit.Var(c[2], 0, color),
                     Bit.Var(c[3], 0, color),
                     Bit.Var(c[0], 1, color),
                     Bit.Var(c[1], 1, color),
                     Bit.Var(c[2], 1, color),
                     Bit.Var(c[3], 1, color),
                     Bit.Var(c[0], 2, color),
                     Bit.Var(c[1], 2, color),
                     Bit.Var(c[2], 2, color),
                     Bit.Var(c[3], 2, color),
                     Bit.Var(c[0], 3, color),
                     Bit.Var(c[1], 3, color),
                     Bit.Var(c[2], 3, color),
                     Bit.Var(c[3], 3, color)])

    A = Array.Make(MakeWord)
    N = len(A)
    Array.Print("Data:", *A)

    B = Array.Zero(N, 16)
    Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
         N / 4, 2, Channel(A, 2, 2), Channel(A, 0, 2), Channel(B, 0, 2))
    Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
         N / 4, 2, Channel(A, 0, 2), Channel(A, 2, 2), Channel(B, 2, 2))
    Array.Print("Swap 8x4:", *B)

    C = [Array.Zero(N / 4, 16) for i in range(4)]
    Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
         N / 4, 1, Channel(B, 1, 3), Channel(B, 0, 3), Channel(C[0]))
    Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
         N / 4, 1, Channel(B, 3, 3), Channel(B, 2, 3), Channel(C[2]))
    Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
         N / 4, 1, Channel(B, 0, 3), Channel(B, 1, 3), Channel(C[1]))
    Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
         N / 4, 1, Channel(B, 2, 3), Channel(B, 3, 3), Channel(C[3]))
    print("Bitplanes:")
    Array.Print("[0]:", *C[0])
    Array.Print("[1]:", *C[1])
    Array.Print("[2]:", *C[2])
    Array.Print("[3]:", *C[3])


c2p()
