#!/usr/bin/env python3

from array import array

data = array('B')

with open("JazzCat-RitchieGlitchie.smp", "rb") as f:
    data.frombytes(f.read())
    data.append(0)

out = array('B')
for i in range(len(data) - 1):
    out.append((data[i] - data[i - 1]) & 255)

with open("JazzCat-RitchieGlitchie.smp.delta", "wb") as f:
    f.write(out)
