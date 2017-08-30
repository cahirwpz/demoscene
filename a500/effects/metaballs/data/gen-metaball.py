#!/usr/bin/env python -B

import Image
from utils import dist, lerp, sq, constrain


if __name__ == "__main__":
  D = 0.9
  size = (80, 80)
  data = []
  pal = []

  for i in range(8):
    pal.extend([0, 0, 0])
  for i in range(16):
    c = int(lerp(0, 255, float(i + 1) / 16))
    pal.extend([c, c, c])
  for i in range(4):
    pal.extend([255, 255, 255])
  for i in range(4):
    c = int(lerp(255, 0, float(i + 1) / 4))
    pal.extend([c, c, c])

  for i in range(size[0]):
    for j in range(size[1]):
      x = lerp(-D, D, float(i) / size[0])
      y = lerp(-D, D, float(j) / size[1])

      d = dist(x, y, 0, 0);

      if d < D:
        p = constrain(int(sq(1.0 - d) * 128), 0, 31)
      else:
        p = 0

      data.append(int(p))

  im = Image.new('L', size)
  im.putdata(data)
  im.putpalette(pal)
  im.save("metaball.png", "PNG")
