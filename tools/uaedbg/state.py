def unique():
    number = 0
    while True:
        number += 1
        yield number


class BreakPoint():
    unique_number = unique()

    def __init__(self, address):
        self.number = self.unique_number.send(None)
        self.address = address

    def __lt__(self, other):
        return self.number < other.number

    def __str__(self):
        return '#%d at %08X' % (self.number, self.address)


class Registers():
    names = ['D0', 'D1', 'D2', 'D3', 'D4', 'D5', 'D6', 'D7',
             'A0', 'A1', 'A2', 'A3', 'A4', 'A5', 'A6', 'A7',
             'PC', 'USP', 'ISP', 'SR']

    def __init__(self, **kwargs):
        self._regs = {}
        for n in self.names:
            self._regs[n] = kwargs.get(n, 0)

    def __getitem__(self, name):
        return self._regs[name]

    def __setitem__(self, name, value):
        self._regs[name] = int(value)

    def _printable(self, name):
        val = self._regs.get(name)
        if name == 'SR':
            return '{}={:04X}'.format(name, val)
        return '{}={:08X}'.format(name, val)

    def __str__(self):
        l0 = ' '.join(self._printable(n) for n in self.names[:8])
        l1 = ' '.join(self._printable(n) for n in self.names[8:16])
        l2 = ' '.join(self._printable(n) for n in self.names[16:])
        return '\n'.join([l0, l1, l2])
