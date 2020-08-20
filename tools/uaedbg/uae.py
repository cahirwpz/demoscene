import asyncio
import signal

from .info import Segment
from .state import Registers


def ParseStatusRegister(line):
    T, S, M, X, N, Z, V, C, IMASK, STP = \
            [f.split('=')[1] for f in line.split()]
    SR_HI = '{:02b}{}{}0{:03b}'.format(int(T), S, M, int(IMASK))
    SR_LO = '000{}{}{}{}{}'.format(X, N, Z, V, C)
    return int(SR_HI + SR_LO, 2)


def ParseProcessorState(lines):
    regs = Registers()
    lines = [line.strip() for line in lines]

    # Read general purpose and supervisor registers,
    # until you hit Status Register info.
    # 'D0 000424B9   D1 00000000   D2 00000000   D3 00000000'
    # 'D4 00000000   D5 00000000   D6 FFFFFFFF   D7 00000000'
    # 'A0 00CF6D1C   A1 00DC0000   A2 00D40000   A3 00000000'
    # 'A4 00D00000   A5 00FC0208   A6 00C00276   A7 00040000'
    # 'USP  00000000 ISP  00040000'
    while not lines[0].startswith('T='):
        fs = lines.pop(0).split()
        for n, v in zip(fs[0::2], fs[1::2]):
            regs[n] = int(v, 16)

    # We are at line starting with 'T=' so read Status Register.
    # 'T=00 S=1 M=0 X=0 N=0 Z=1 V=0 C=0 IMASK=7 STP=0'
    regs['SR'] = ParseStatusRegister(lines.pop(0))

    # floating point unit registers follow?
    while lines[0].startswith('FP'):
        lines.pop(0)

    # Only for 68000:
    # 'Prefetch fffc (ILLEGAL) 51c8 (DBcc) Chip latch 00000000'
    if lines[0].startswith('Prefetch'):
        lines.pop(0)

    # Only for 68030 + MMU:
    # 'SRP: 0 CRP: 800000020781C000'
    # 'TT0: 00000000 TT1: 00000000 TC: 80C07760'
    if lines[0].startswith('SRP:'):
        lines.pop(0)

    if lines[0].startswith('TT0:'):
        lines.pop(0)

    # '00FC0610 51c8 fffc  DBF .W D0,#$fffc == $00fc060e (F)'
    regs['PC'] = int(lines.pop(0).split()[0], 16)

    # 'Next PC: 00fc0614'
    assert(lines.pop(0).startswith('Next PC'))

    return regs


class DisassemblyLine():
    def __init__(self, address, opcode, mnemonic):
        self.address = address
        self.opcode = opcode
        self.mnemonic = mnemonic

    @property
    def next_address(self):
        return self.address + len(self.opcode) / 2

    def __str__(self):
        return '%08X %-32s %s' % (self.address, self.opcode, self.mnemonic)


class UaeProcess():
    def __init__(self, proc):
        self.proc = proc

    @property
    def reader(self):
        return self.proc.stderr

    @property
    def writer(self):
        return self.proc.stdin

    def interrupt(self):
        self.proc.send_signal(signal.SIGINT)

    def terminate(self):
        self.proc.send_signal(signal.SIGKILL)

    async def wait(self):
        return await self.proc.wait()

    async def communicate(self, cmd):
        self.send(cmd)
        return await self.recv()

    def send(self, cmd):
        self.writer.write(cmd.encode() + b'\n')

    async def recv(self):
        text = ''

        while True:
            try:
                raw_text = await self.reader.readuntil(b'>')
            except asyncio.IncompleteReadError as ex:
                raise EOFError
            text += raw_text.decode()
            # finished by debugger prompt ?
            if text.endswith('\n>'):
                text = text[:-2]
                return [line.rstrip() for line in text.splitlines()]

    async def logger(self):
        try:
            while not self.proc.stdout.at_eof():
                raw_text = await self.proc.stdout.readline()
                print(raw_text.decode(), end='')
        except asyncio.CancelledError:
            pass
        except asyncio.IncompleteReadError:
            pass

    def resume(self):
        self.send('g')

    def step(self):
        self.send('t')

    async def insert_hwbreak(self, addr):
        lines = await self.communicate('f %X' % addr)
        return lines and lines[0] == 'Breakpoint added'

    async def remove_hwbreak(self, addr):
        lines = await self.communicate('f %X' % addr)
        return lines and lines[0] == 'Breakpoint removed'

    async def read_registers(self):
        lines = await self.communicate('r')
        return ParseProcessorState(lines)

    async def read_memory(self, addr, length):
        # 00000004 00C0 0276 00FC 0818 00FC 081A 00FC 081C  ...v............'
        # 00000014 00FC 081E 00FC 0820 00FC 0822 00FC 090E  ....... ..."....'
        # ...
        lines = await self.communicate('m %x %d' % (addr, (length + 15) / 16))
        hexlines = [''.join(line.split()[1:9]) for line in lines]
        return ''.join(hexlines)[:length*2]

    async def read_byte(self, addr):
        return int(await self.read_memory(addr, 1), 16)

    async def read_word(self, addr):
        return int(await self.read_memory(addr, 2), 16)

    async def read_long(self, addr):
        return int(await self.read_memory(addr, 4), 16)

    async def disassemble(self, addr, n=1):
        # 00FC10BC 33fc 4000 00df f09a      MOVE.W #$4000,$00dff09a
        lines = await self.communicate('d %x %d' % (addr, n))
        disassembly = []
        for line in lines:
            pc = int(line[:8].strip(), 16)
            op = ''.join(line[8:34].strip().split()).upper()
            ins = line[34:].strip()
            disassembly.append(DisassemblyLine(pc, op, ins))
        return disassembly

    async def prologue(self):
        lines = await self.recv()
        data = {}
        # Breakpoint at 00C04EB0
        while lines[0].startswith('Breakpoint'):
            line = lines.pop(0)
            data['break'] = int(line.split()[2], 16)
        # Exception 27, PC=00C15AC6
        while lines[0].startswith('Exception'):
            line = lines.pop(0)
            data['exception'] = int(line[10:].split(',')[0])
        # just processor state
        data['regs'] = ParseProcessorState(lines)
        return data

    async def fetch_segments(self):
        # assume for now that VBR is at 0
        vbr = 0
        magic = await self.read_long(0)
        if magic != 0x1ee7c0de:
            return None
        segments = []
        seg = await self.read_long(4)
        while seg != 0:
            start = seg + 8
            size = await self.read_long(seg) - 8
            seg = await self.read_long(seg + 4)
            segments.append(Segment(start, size))
        return segments
