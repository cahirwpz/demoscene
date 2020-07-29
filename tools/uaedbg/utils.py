def to_ascii(a):
    b = bytearray(a)
    for i in range(len(b)):
        if b[i] < 32 or b[i] >= 128:
            b[i] = 46
    return b.decode('ascii')


def hexdump(data):
    data = bytearray(data)
    hexch = ['%.2X' % b for b in data]
    ascii = to_ascii(data)

    for i in range(0, len(hexch), 16):
        hexstr = ' '.join(hexch[i:i + 16])
        asciistr = ''.join(ascii[i:i + 16])
        print('  {2:04} | {0:<47} |{1}|'.format(hexstr, asciistr, i))
