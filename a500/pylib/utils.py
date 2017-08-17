from math import sqrt, floor


def frpart(x):
    return x - floor(x)


def constrain(val, lo, hi):
    if val < lo:
        return lo
    if val > hi:
        return hi
    return val


def sq(x):
    return x * x


def saw(x):
    t = x - floor(x)
    if t > 0.5:
        return 2.0 * (1.0 - t)
    else:
        return 2.0 * t


def lerp(lo, hi, step):
    return lo + (hi - lo) * step


def dist(x1, y1, x2, y2):
    dx = x2 - x1
    dy = y2 - y1
    return sqrt(dx * dx + dy * dy)


def ccir601(rgb):
    return 0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]
