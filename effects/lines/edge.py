#!/usr/bin/env python3

def edge(x1, y1, x2, y2):
    if y1 > y2:
        xt = x1
        x1 = x2
        x2 = xt
        yt = y1
        y1 = y2
        y2 = yt

    dx = x2 - x1
    dy = y2 - y1

    if dy == 0:
        return

    di = dx // dy
    df = abs(dx) % dy
    xi = 0
    xf = 0
    s = [-1, 1][dx >= 0]

    while y1 <= y2:
        print('{} {}'.format(xi+x1, y1))
        y1 += 1
        xi += di
        xf += df
        if xf >= dy:
            xf -= dy
            xi += s


edge(16, 0, 48, 64)
