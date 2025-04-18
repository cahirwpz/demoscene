#!/usr/bin/env python3


from common import Bit, Word, Channel, Blit, Array


def c2p(bitplane_output=True):
    m0 = Word.Mask('00ff')
    m1 = Word.Mask('0f0f')
    m2 = Word.Mask('3333')
    m3 = Word.Mask('5555')

    print("=[ c2p 1x1 4bpl (4 sprites for 64px wide sprite) ]="
          .center(48, '-'))

    def MakeWord(chars, color):
        bits = []
        for c in chars:
            for i in range(4):
                bits.append(Bit.Var(c, i, color))
        return Word(bits)

    A = Array.Make(MakeWord)
    N = len(A)
    Array.Print("Data:", *A)
    # Channel(data, start, modulo)
    B = Array.Zero(N, 16)
    Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
         N // 4, 2, Channel(A, 2, 2), Channel(A, 0, 2), Channel(B, 0, 2))
    Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
         N // 4, 2, Channel(A, 0, 2), Channel(A, 2, 2), Channel(B, 2, 2))
    Array.Print("Swap 8x4:", *B)

    C = Array.Zero(N, 16)
    Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
         N // 2, 1, Channel(B, 1, 1), Channel(B, 0, 1), Channel(C, 0, 1))
    Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
         N // 2, 1, Channel(B, 0, 1), Channel(B, 1, 1), Channel(C, 1, 1))
    Array.Print("Swap 4x2:", *C)

    D = Array.Zero(N, 16)
    Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
         N // 4, 2, Channel(C, 2, 2), Channel(C, 0, 2), Channel(D, 0, 2))
    Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
         N // 4, 2, Channel(C, 0, 2), Channel(C, 2, 2), Channel(D, 2, 2))
    Array.Print("Swap 2x2:", *D)

    if bitplane_output:
        E = [Array.Zero(N // 4, 16) for i in range(4)]
        Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3), N // 4, 1,
             Channel(D, 1, 3), Channel(D, 0, 3), Channel(E[0], 0, 0))
        Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3), N // 4, 1,
             Channel(D, 3, 3), Channel(D, 2, 3), Channel(E[2], 0, 0))
        Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3), N // 4, 1,
             Channel(D, 0, 3), Channel(D, 1, 3), Channel(E[1], 0, 0))
        Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3), N // 4, 1,
             Channel(D, 2, 3), Channel(D, 3, 3), Channel(E[3], 0, 0))

        print("Bitplanes:")
        Array.Print("[0]:", *E[0])
        Array.Print("[1]:", *E[1])
        Array.Print("[2]:", *E[2])
        Array.Print("[3]:", *E[3])
    else:
        E = Array.Zero(N, 16)
        Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3),
             N // 2, 1, Channel(D, 1, 1), Channel(D, 0, 1), Channel(E, 0, 1))
        Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3),
             N // 2, 1, Channel(D, 0, 1), Channel(D, 1, 1), Channel(E, 1, 1))
        Array.Print("Swap 1x1:", *E)

        SPR0DATA = Array.Zero(N//4, 16)
        SPR0DATB = Array.Zero(N//4, 16)
        SPR1DATA = Array.Zero(N//4, 16)
        SPR1DATB = Array.Zero(N//4, 16)
        print(f"N={N}")
        Blit(lambda a, b: a,
             N//4, 1, Channel(E, 0, 3), Channel(E), Channel(SPR0DATA, 0, 0))
        Blit(lambda a, b: a,
             N//4, 1, Channel(E, 1, 3), Channel(E), Channel(SPR0DATB, 0, 0))
        Blit(lambda a, b: a,
             N//4, 1, Channel(E, 2, 3), Channel(E), Channel(SPR1DATA, 0, 0))
        Blit(lambda a, b: a,
             N//4, 1, Channel(E, 3, 3), Channel(E), Channel(SPR1DATB, 0, 0))
        Array.Print("SPR0DATA word wise", *SPR0DATA)
        Array.Print("SPR0DATB word wise", *SPR0DATB)
        Array.Print("SPR1DATA word wise", *SPR1DATA)
        Array.Print("SPR1DATB word wise", *SPR1DATB)


# Do c2p with target output being sprite data
c2p(bitplane_output=False)
