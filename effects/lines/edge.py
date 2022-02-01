#!/usr/bin/env python3

def edge(xs, ys, xe, ye):
    print(f'\nedge: ({xs}, {ys}) -> {xe} {ye}')

    if ys > ye:
        xt = xs
        xs = xe
        xe = xt
        yt = ys
        ys = ye
        ye = yt

    dx = xe - xs
    dy = ye - ys

    if dy == 0:
        return

    di = abs(dx) // dy
    df = abs(dx) % dy

    if dx >= 0:
        s = 1
    else:
        s = -1
        di = -di

    xi = 0
    xf = 0

    while ys <= ye:
        x = xi+xs
        wi = x >> 4
        wb = 1 << (15 - (x & 15))
        print(f'{x:2} ({wi:02}:{wb:04x}) {ys:2}')
        ys += 1
        xi += di
        xf += df
        if xf >= dy:
            xf -= dy
            xi += s


edge(8, 0, 56, 32)
edge(56, 0, 8, 32)
