#!/usr/bin/env python -B

from common import Bit, Word, Array


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

    Array.Print("Data:", a0, a1, a2, a3)

    b0 = (a0 & m1) | (a2.lsr(2) & m0)
    b1 = (a1 & m1) | (a3.lsr(2) & m0)
    b2 = (a0.lsl(2) & m1) | (a2 & m0)
    b3 = (a1.lsl(2) & m1) | (a3 & m0)

    Array.Print("Swap 2x2:", b0, b1, b2, b3)

    c0 = (b0 & m3) | (b1.lsr(1) & m2)
    c1 = (b0.lsl(1) & m3) | (b1 & m2)
    c2 = (b2 & m3) | (b3.lsr(1) & m2)
    c3 = (b2.lsl(1) & m3) | (b3 & m2)

    Array.Print("Swap 1x1:", c0, c1, c2, c3)


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

    Array.Print("Data:", a0, a1, a2, a3, a4, a5, a6, a7)

    b0 = (a0 & m1) | (a4.lsr(4) & m0)
    b1 = (a1 & m1) | (a5.lsr(4) & m0)
    b2 = (a2 & m1) | (a6.lsr(4) & m0)
    b3 = (a3 & m1) | (a7.lsr(4) & m0)
    b4 = (a0.lsl(4) & m1) | (a4 & m0)
    b5 = (a1.lsl(4) & m1) | (a5 & m0)
    b6 = (a2.lsl(4) & m1) | (a6 & m0)
    b7 = (a3.lsl(4) & m1) | (a7 & m0)

    Array.Print("Swap 4x4:", b0, b1, b2, b3, b4, b5, b6, b7)

    c0 = (b0 & m3) | (b2.lsr(2) & m2)
    c1 = (b1 & m3) | (b3.lsr(2) & m2)
    c2 = (b0.lsl(2) & m3) | (b2 & m2)
    c3 = (b1.lsl(2) & m3) | (b3 & m2)
    c4 = (b4 & m3) | (b6.lsr(2) & m2)
    c5 = (b5 & m3) | (b7.lsr(2) & m2)
    c6 = (b4.lsl(2) & m3) | (b6 & m2)
    c7 = (b5.lsl(2) & m3) | (b7 & m2)

    Array.Print("Swap 2x2:", c0, c1, c2, c3, c4, c5, c6, c7)

    d0 = (c0 & m5) | (c1.lsr(1) & m4)
    d1 = (c0.lsl(1) & m5) | (c1 & m4)
    d2 = (c2 & m5) | (c3.lsr(1) & m4)
    d3 = (c2.lsl(1) & m5) | (c3 & m4)
    d4 = (c4 & m5) | (c5.lsr(1) & m4)
    d5 = (c4.lsl(1) & m5) | (c5 & m4)
    d6 = (c6 & m5) | (c7.lsr(1) & m4)
    d7 = (c6.lsl(1) & m5) | (c7 & m4)

    Array.Print("Swap 1x1:", d0, d1, d2, d3, d4, d5, d6, d7)


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

    Array.Print("Data:", i0, i1, i2, i3)

    a0 = (i0.lsl(4) & m0) | (i1 & m1)
    a2 = (i2.lsl(4) & m0) | (i3 & m1)
    a1 = (i0 & m0) | (i1.lsr(4) & m1)
    a3 = (i2 & m0) | (i3.lsr(4) & m1)

    Array.Print("Swap 4x2:", a0, a1, a2, a3)

    b0 = a0 & m3
    b2 = a1 & m3
    b4 = a2 & m3
    b6 = a3 & m3

    b1 = a0.lsr(1) & m3
    b3 = a1.lsr(1) & m3
    b5 = a2.lsr(1) & m3
    b7 = a3.lsr(1) & m3

    Array.Print("Bitplanes:", b0, b1, b2, b3, b4, b5, b6, b7)

    b0 = (a0 & m3) | (a0.lsl(1) & m2)
    b2 = (a1 & m3) | (a1.lsl(1) & m2)
    b4 = (a2 & m3) | (a2.lsl(1) & m2)
    b6 = (a3 & m3) | (a3.lsl(1) & m2)

    b1 = (a0.lsr(1) & m3) | (a0 & m2)
    b3 = (a1.lsr(1) & m3) | (a1 & m2)
    b5 = (a2.lsr(1) & m3) | (a2 & m2)
    b7 = (a3.lsr(1) & m3) | (a3 & m2)

    Array.Print("Bitplanes:", b0, b1, b2, b3, b4, b5, b6, b7)


# c2p_2_1_4_mangled()
c2p_2_1_4_stingray()
# c2p_2_1_4()
