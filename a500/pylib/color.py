from collections import namedtuple
from utils import lerp


class RGB(namedtuple('RGB', 'r g b')):
    """ stores color as a integer triple from range [0, 255] """


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
