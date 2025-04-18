#!/usr/bin/env python3

from PIL import Image


# reduce colors to 4-bit components
def rgb12(pix, w, h):
    for y in range(h):
        for x in range(w):
            r, g, b = pix[x, y]
            pix[x, y] = (r & 0xf0, g & 0xf0, b & 0xf0)


img = Image.open("tree-24.png")
w, h = img.size
pix = img.load()

for N in range(1, 8):
    K = 32 + 13 * (N - 1)

    print("pass #%d: make each %d consecutive lines share no more than "
          "%d colors." % (N, N, K))

    rgb12(pix, w, h)

    for y in range(N, h):
        line = img.crop((0, y - N, w, y + 1))
        line = line.convert("P", dither=Image.NONE, palette=Image.ADAPTIVE,
                            colors=K)
        line = line.convert("RGB")
        line = line.load()
        for k in range(N):
            for x in range(w):
                pix[x, y + (k - N)] = line[x, k]


rgb12(pix, w, h)
img.save("tree-12.png")
