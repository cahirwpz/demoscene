#!/usr/bin/env python3

from PIL import Image
import sys


def println(stmt, *args):
  print("\t%s\t%s" % (stmt, ", ".join(str(arg) for arg in args)))


def main():
  img = Image.open(sys.argv[1])
  img.load()

  frames = 0

  for i in range(65536):
    try:
      img.seek(i)
      frames += 1
    except EOFError:
      break

  print("; vim:ft=asm68k")
  println("section", "data", "data_p")

  println("dc.w", *img.size)
  println("dc.w", 0, frames)
  for n in range(frames):
    println("dc.l", ".frame%d" % n)

  for n in range(frames):
    print(".frame%d:" % n)
    img.seek(n)
    pix = img.load()
    for y in range(img.size[1]):
      line = []
      p = 1
      for x in range(img.size[0]):
        if p != pix[x, y]:
          p = pix[x, y]
          line.append(x)
      println("dc.b", len(line), *line)


if __name__ == '__main__':
  main()
