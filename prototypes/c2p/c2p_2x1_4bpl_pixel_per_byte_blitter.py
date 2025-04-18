#!/usr/bin/env python3

from common import Bit, Word, Array, Blit, Channel


def c2p(bitplane_output=True):
    print("=[ c2p 2x1 4bpl (pixel per byte) ]=".center(48, '-'))

    # premangled pixels:
    # [- - - - a b c d] => [a b - - c d - -]
    def MakeWord(c, color):
        return Word([Bit.Var(c[0], 0, color),
                     Bit.Var(c[0], 1, color),
                     Bit.Const(0, color),
                     Bit.Const(0, color),
                     Bit.Var(c[0], 2, color),
                     Bit.Var(c[0], 3, color),
                     Bit.Const(0, color),
                     Bit.Const(0, color),
                     Bit.Var(c[1], 0, color),
                     Bit.Var(c[1], 1, color),
                     Bit.Const(0, color),
                     Bit.Const(0, color),
                     Bit.Var(c[1], 2, color),
                     Bit.Var(c[1], 3, color),
                     Bit.Const(0, color),
                     Bit.Const(0, color)])

    def ArrayMake(fn):
        return [fn("ab", color="31m"),
                fn("cd", color="31m"),
                fn("ef", color="32m"),
                fn("gh", color="32m"),
                fn("ij", color="33m"),
                fn("kl", color="33m"),
                fn("mn", color="34m"),
                fn("op", color="34m"),
                fn("qr", color="35m"),
                fn("st", color="35m"),
                fn("uv", color="36m"),
                fn("wx", color="36m"),
                fn("AB", color="31;1m"),
                fn("CD", color="31;1m"),
                fn("EF", color="32;1m"),
                fn("GH", color="32;1m"),
                fn("IJ", color="33;1m"),
                fn("KL", color="33;1m"),
                fn("MN", color="34;1m"),
                fn("OP", color="34;1m"),
                fn("QR", color="35;1m"),
                fn("ST", color="35;1m"),
                fn("UV", color="36;1m"),
                fn("WX", color="36;1m")]

    # size in bytes: W * H
    A = ArrayMake(MakeWord)
    N = len(A)
    Array.Print("Data:", *A)

    m0 = Word.Mask('3300')
    m1 = Word.Mask('ff00')
    m2 = Word.Mask('f0f0')
    m3 = Word.Mask('aaaa')

    B = Array.Zero(N, 16)
    Blit(lambda a, b: ((a << 6) & m0) | (b & ~m0),
         N, 1, Channel(A), Channel(A), Channel(B))
    Array.Print("[0]:", *B)

    C = Array.Zero(N // 2, 16)
    Blit(lambda a, b: (a & m1) | ((b >> 8) & ~m1),
         N // 4, 1, Channel(B, 0, 3), Channel(B, 2, 3), Channel(C, 0, 1))
    Blit(lambda a, b: (a & m1) | ((b >> 8) & ~m1),
         N // 4, 1, Channel(B, 1, 3), Channel(B, 3, 3), Channel(C, 1, 1))
    Array.Print("[0]:", *C)

    D = [Array.Zero(N // 4, 16) for i in range(2)]
    Blit(lambda a, b: (a & m2) | ((b >> 4) & ~m2),
         N // 4, 1, Channel(C, 0, 1), Channel(C, 1, 1), Channel(D[0]))
    Blit(lambda a, b: ((a << 4) & m2) | (b & ~m2),
         N // 4, 1, Channel(C, 0, 1), Channel(C, 1, 1), Channel(D[1]))
    print("Swap 4x2:")
    Array.Print("[0]:", *D[0])
    Array.Print("[1]:", *D[1])

    E = [Array.Zero(N // 4, 16) for i in range(4)]
    Blit(lambda a, b: (a & m3) | ((b >> 1) & ~m3),
         N // 4, 1, Channel(D[0]), Channel(D[0]), Channel(E[0]))
    Blit(lambda a, b: (a & m3) | ((b >> 1) & ~m3),
         N // 4, 1, Channel(D[1]), Channel(D[1]), Channel(E[2]))
    Blit(lambda a, b: ((a << 1) & m3) | (b & ~m3),
         N // 4, 1, Channel(D[0]), Channel(D[0]), Channel(E[1]))
    Blit(lambda a, b: ((a << 1) & m3) | (b & ~m3),
         N // 4, 1, Channel(D[1]), Channel(D[1]), Channel(E[3]))
    print("Expand 2x1:")
    Array.Print("[0]:", *E[0])
    Array.Print("[1]:", *E[1])
    Array.Print("[2]:", *E[2])
    Array.Print("[3]:", *E[3])


c2p()
