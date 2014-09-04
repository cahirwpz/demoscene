#!/usr/bin/env python -B

import Image
import sys
import os
from array import array


if __name__ == "__main__":
  image = Image.open(sys.argv[1])
  image = image.convert("L")

  bumpmap = array("H")
  name = os.path.splitext(sys.argv[1])[0] + ".bin"

  for j in range(image.size[1]):
    for i in range(image.size[0]):
      this = image.getpixel((i, j))
      if j < image.size[1] - 1:
        down = image.getpixel((i, j + 1))
      else:
        down = this
      if i < image.size[0] - 1:
        right = image.getpixel((i + 1, j))
      else:
        right = this
      u = ((down - this + 128) / 2) & 127
      v = ((right - this + 128) / 2) & 127
      bumpmap.append(u * 128 + v)

  with open(name, "w") as f:
    bumpmap.tofile(f)
