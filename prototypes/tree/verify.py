#!/usr/bin/env python3

from PIL import Image

img = Image.open("tree-12.png")
w, h = img.size
pix = img.load()

lines = []
for y in range(h):
    line = set(pix[x, y] for x in range(w))
    n = len(line)
    if n > 32:
        print("line %d has %d colors" % (y, n))
    lines.append(line)

for y in range(h - 1):
    l1 = lines[y]
    l2 = lines[y + 1]
    d = len(l1.difference(l2))
    if d > 13:
        print("line %d..%d: change in %d colors" % (y, y + 1, d))

colors = set()
for line in lines:
    colors.update(line)
print("image uses %d colors" % len(colors))
