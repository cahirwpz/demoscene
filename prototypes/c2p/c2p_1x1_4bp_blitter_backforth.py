#!/usr/bin/env python3
# Blitter C2P using two memory locations (back-and-forth
# bit shuffling)


from common import Bit, Word, Channel, Blit, Array


def c2p(bitplane_output=True):
    m0 = Word.Mask('00ff')
    m1 = Word.Mask('0f0f')
    m2 = Word.Mask('3333')
    m3 = Word.Mask('5555')

    print("=[ c2p 1x1 4bpl (blitter) ]=".center(48, '-'))

    def MakeWord(chars, color):
        bits = []
        for c in chars:
            for i in range(4):
                bits.append(Bit.Var(c, i, color))
        return Word(bits)

    chunky = Array.Make(MakeWord)
    N = len(chunky)
    Array.Print("Chunky:", *chunky)

    planes = Array.Zero(N, 16)
    Blit(lambda a, b: ((a >> 8) & m0) | (b & ~m0),
         N // 4, 2, Channel(chunky, 2, 2), Channel(chunky, 0, 2),
         Channel(planes, 0, 2))
    Array.Print("Swap 8x4: planar after 1st pass:", *planes)

    Blit(lambda a, b: ((a << 8) & ~m0) | (b & m0),
         N // 4, 2, Channel(chunky, 0, 2), Channel(chunky, 2, 2),
         Channel(planes, 2, 2))
    Array.Print("Swap 8x4: planar:", *planes)

    Blit(lambda a, b: ((a >> 4) & m1) | (b & ~m1),
         N // 2, 1, Channel(planes, 1, 1), Channel(planes, 0, 1),
         Channel(chunky, 0, 1))

    Array.Print("Swap 4x2 chunky after 1st pass:", *chunky)

    Blit(lambda a, b: ((a << 4) & ~m1) | (b & m1),
         N // 2, 1, Channel(planes, 0, 1), Channel(planes, 1, 1),
         Channel(chunky, 1, 1))
    Array.Print("Swap 4x2: chunky:", *chunky)

    Blit(lambda a, b: ((a >> 2) & m2) | (b & ~m2),
         N // 4, 2, Channel(chunky, 2, 2), Channel(chunky, 0, 2),
         Channel(planes, 0, 2))

    Array.Print("Swap 2x2: planar after 1st pass:", *planes)

    Blit(lambda a, b: ((a << 2) & ~m2) | (b & m2),
         N // 4, 2, Channel(chunky, 0, 2), Channel(chunky, 2, 2),
         Channel(planes, 2, 2))

    Array.Print("Swap 2x2: planar:", *planes)

    if bitplane_output:
        E = [Array.Zero(N // 4, 16) for i in range(4)]
        Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3), N // 4, 1,
             Channel(planes, 1, 3), Channel(planes, 0, 3), Channel(E[0], 0, 0))
        Blit(lambda a, b: ((a >> 1) & m3) | (b & ~m3), N // 4, 1,
             Channel(planes, 3, 3), Channel(planes, 2, 3), Channel(E[2], 0, 0))
        Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3), N // 4, 1,
             Channel(planes, 0, 3), Channel(planes, 1, 3), Channel(E[1], 0, 0))
        Blit(lambda a, b: ((a << 1) & ~m3) | (b & m3), N // 4, 1,
             Channel(planes, 2, 3), Channel(planes, 3, 3), Channel(E[3], 0, 0))

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


c2p()
