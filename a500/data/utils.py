from math import sqrt


def constrain(val, lo, hi):
  if val < lo:
    return lo
  if val > hi:
    return hi
  return val


def sq(x):
  return x * x


def lerp(lo, hi, step):
  return lo + (hi - lo) * step


def dist(x1, y1, x2, y2):
  dx = x2 - x1
  dy = y2 - y1
  return sqrt(dx * dx + dy * dy)
