from math import floor
from collections import namedtuple
from utils import lerp


class RGB(namedtuple('RGB', 'r g b')):
    """ stores color as a integer triple from range [0, 255] """


class HSV(namedtuple('HSV', 'h s v')):
    """
        stores (hue, saturation, value) color,
        each component is in range [0.0, 1.0]
    """

    @classmethod
    def fromColor(cls, c):
        c_max = max(c)
        c_min = min(c)
        diff = c_max - c_min

        h, s, v = 0.0, 0.0, c_max

        if c_max > 0.0:
            s = diff / c_max

        if s != 0.0:
            if c.r == c_max:
                h = (c.g - c.b) / diff
            elif c.g == c_max:
                h = 2.0 + (c.b - c.r) / diff
            else:
                h = 4.0 + (c.r - c.g) / diff

            h /= 6.0

            if h < 0.0:
                h += 1.0

        return cls(h, s, v)

    def toColor(self):
        h, s, v = self.h, self.s, self.v

        if s == 0.0:
            return Color(v, v, v)

        if h == 1.0:
            h = 0.0

        h *= 6.0

        hi = floor(h)
        hf = h - hi

        p = v * (1.0 - s)
        q = v * (1.0 - s * hf)
        t = v * (1.0 - s * (1.0 - hf))

        if hi == 0:
            return Color(v, t, p)

        if hi == 1:
            return Color(q, v, p)

        if hi == 2:
            return Color(p, v, t)

        if hi == 3:
            return Color(p, q, v)

        if hi == 4:
            return Color(t, p, v)

        return Color(v, p, q)


class Color(namedtuple('Color', 'r g b')):
    """ stores color as a float triple from range [0.0, 1.0] """

    def rgb12(self):
        r = int(self.r * 255.0)
        g = int(self.g * 255.0)
        b = int(self.b * 255.0)
        r = (r & ~15) | (r >> 4)
        g = (g & ~15) | (g >> 4)
        b = (b & ~15) | (b >> 4)
        return RGB(r, g, b)

    def rgb24(self):
        r = int(self.r * 255.0)
        g = int(self.g * 255.0)
        b = int(self.b * 255.0)
        return RGB(r, g, b)

    @staticmethod
    def lerp(lo, hi, step):
        r = lerp(lo.r, hi.r, step)
        g = lerp(lo.g, hi.g, step)
        b = lerp(lo.b, hi.b, step)
        return Color(r, g, b)
