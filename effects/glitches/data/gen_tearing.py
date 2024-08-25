#!/usr/bin/env python3

from contextlib import redirect_stdout
import sys
import random

arr = [[0] * 16 for i in range(16)]


def generateRandomTable():
    o = random.randint(5, 16)
    u = random.randint(0, 2)

    print(o)
    bby = [[0] * o for i in range(o)]

    for i in range(o):
        for j in range(o):
            if j < i:
                h = hex((i-j)*17)
                bby[i][o-j-1] = h
        u = bby[i].copy()
        u.reverse()

        b = bby[i] + u
        bby[i] = b
    return bby


def generateTable():
    arr = [[0] * 16 for i in range(16)]

    for i in range(16):
        for j in range(16):
            if j < i:
                value = i - j + random.randint(-5, 5)
                if value <= 0:
                    value = 0
                    arr[i][15-j] = value
                else:
                    arr[i][15-j] = hex(value*17)
        u = arr[i].copy()
        u.reverse()
        # print(u)
        b = arr[i] + u
        arr[i] = b
    return arr


def saveFile(bby):
    with open(sys.argv[1], 'w') as f:
        with redirect_stdout(f):
            print("static __data u_short tears[16][32] = {")
            for aa in bby:
                print("    {", end=" ")
                for a in aa:
                    if a == 0:
                        print("0x00,", end=" ")
                    else:
                        print(f"{a},", end=" ")
                print("},")
            print('};\n')


bby = generateTable()
saveFile(bby)
