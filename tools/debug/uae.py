#!/usr/bin/env python3

import asyncio
import logging
import signal

from prompt_toolkit.patch_stdout import patch_stdout
from prompt_toolkit.shortcuts import PromptSession
from prompt_toolkit.history import InMemoryHistory


CUSTOM = 0xdff000
CUSTOM_SIZE = 0x200


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

    def as_hex(self, name):
        val = self._regs.get(name, None)
        if val is None:
            return 'XXXXXXXX'
        return '{:08X}'.format(val)

    def __repr__(self):
        regs = ['{}={:08X}'.format(n, self[n]) for n in self.names[:19]]
        regs.append('SR={:08X}'.format(self['SR']))
        return '{' + ' '.join(regs) + '}'


def ParseStatusRegister(line):
    T, S, M, X, N, Z, V, C, IMASK, STP = \
        [f.split('=')[1] for f in line.split()]
    SR_HI = '{:02b}{}{}0{:03b}'.format(int(T), S, M, int(IMASK))
    SR_LO = '000{}{}{}{}{}'.format(X, N, Z, V, C)
    return int(SR_HI + SR_LO, 2)


def ParseProcessorState(lines):
    regs = Registers()
    lines = [line.strip() for line in lines]

    if lines[0].startswith('Cycles'):
        lines.pop(0)

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
    assert lines[-1].startswith('Next PC')

    return regs


class UaeCommandsMixin():

    def resume(self, addr=None):
        # {g [<address>]} Start execution at the current address or <address>.
        cmd = 'g'
        if addr:
            cmd += ' ' + hex(addr)
        self.send(cmd)

    def step(self, insn=None):
        # {t [<instructions>]} Step one or more <instructions>.
        cmd = 't'
        if insn:
            cmd += ' ' + str(insn)
        self.send(cmd)

    def break_opcode(self, opcode):
        # {fi <opcode>} Step forward until PC points to <opcode>.
        self.send('fi {:04x}'.format(int(opcode, 16)))

    async def memory_map(self):
        lines = await self.communicate('dm')
        regions = [(CUSTOM, CUSTOM_SIZE, 'ram')]
        for line in lines:
            line = line.strip()
            if line.startswith('='):
                continue
            start, _1, _2, size, desc = line.split(maxsplit=4)
            segment, blocks = _1.split('/')
            if int(blocks) == 0:
                continue
            if 'memory' in desc:
                desc = 'ram'
            elif 'ROM' in desc:
                desc = 'rom'
            else:
                desc = '???'
            assert segment[-1] == 'K'
            blocks = int(blocks)
            start = int(start, 16)
            segment = int(segment[:-1]) * 1024 // blocks
            for i in range(blocks):
                regions.append((start + segment * i, segment, desc))
        return regions

    async def read_memory(self, addr, length):
        # {m <address> [<lines>]} Memory dump starting at <address>.
        if addr >= CUSTOM and addr < CUSTOM + CUSTOM_SIZE:
            return await self.read_custom(addr - CUSTOM, length)
        lines = await self.communicate('m %x %d' % (addr, (length + 15) / 16))
        # 00000004 00C0 0276 00FC 0818 00FC 081A 00FC 081C  ...v............'
        # 00000014 00FC 081E 00FC 0820 00FC 0822 00FC 090E  ....... ..."....'
        # ...
        if False:
            for line in lines:
                print(line)
        hexlines = [''.join(line.strip().split()[1:9]) for line in lines]
        return ''.join(hexlines)[:length * 2]

    async def read_custom(self, addr, length):
        assert addr + length <= CUSTOM_SIZE
        lines = await self.communicate('e')
        custom = ['0000' for i in range(256)]
        for line in lines:
            fs = line.strip().split()
            i = int(fs[0], 16)
            j = int(fs[3], 16)
            custom[i // 2] = fs[2]
            custom[j // 2] = fs[5]
        custom = ''.join(custom)
        return custom[addr*2:addr*2+length*2]

    async def read_long(self, addr):
        longword = await self.read_memory(addr, 4)
        return int(longword, 16)

    async def write_memory(self, addr, data):
        # {W <address> <values[.x] separated by space>}
        # Write into Amiga memory.
        # Assume _data_ is a string of hexadecimal digits.
        if not data:
            return
        hexbytes = ['%x' % addr]
        while data:
            byte, data = data[:2], data[2:]
            hexbytes.append(byte)
        await self.communicate('W ' + ' '.join(hexbytes))

    # W <address> <values[.x] separated by space> Write into Amiga memory.
    # w <num> <address> <length> <R/W/I/F/C> [<value>[.x]]
    #   (read/write/opcode/freeze/mustchange).
    # Add/remove memory watchpoints.

    async def read_registers(self):
        # {r} Dump state of the CPU.
        return ParseProcessorState(await self.communicate('r'))

    async def write_register(self, regname, value):
        # {r <reg> <value>} Modify CPU registers (Dx,Ax,USP,ISP,VBR,...).
        self.send('r {} {:x}'.format(regname, value))

    async def insert_breakpoint(self, addr):
        # {f <address>} Add/remove breakpoint.
        lines = await self.communicate('f %X' % addr)
        assert lines and lines[0] == 'Breakpoint added.'

    async def remove_breakpoint(self, addr):
        # {f <address>} Add/remove breakpoint.
        lines = await self.communicate('f %X' % addr)
        assert lines and lines[0] == 'Breakpoint removed.'

    async def insert_watchpoint(self, addr, size, kind='I'):
        # w <num> <address> <length> <R/W/I/F/C> [<value>[.x]]
        #   (read/write/opcode/freeze/mustchange).
        # Add/remove memory watchpoints.

        # Watchpoints are deleted by numbers, so we need to maintain the <num>
        # for every watchpoint.
        index = max(self.watchpoints.values(), default=0) + 1
        self.watchpoints[addr, size, kind] = index
        lines = await self.communicate('w %d %X %d %s' %
                                       (index, addr, size, kind))
        assert lines and lines[-1] == 'Memwatch %d added' % index

    async def remove_watchpoint(self, addr, size, kind='I'):
        # w <num> <address> <length> <R/W/I/F/C> [<value>[.x]]
        #   (read/write/opcode/freeze/mustchange).
        # Add/remove memory watchpoints.
        index = self.watchpoints.pop((addr, size, kind))
        lines = await self.communicate('w %d' % index)
        assert lines and lines[-1] == 'Memwatch %d removed' % index

    async def get_segments(self):
        # assume for now that VBR is at 0
        vbr = 0
        magic = await self.read_long(0)
        if magic != 0x1ee7c0de:
            return None
        segments = []
        hunk = await self.read_long(4)
        while True:
            size = await self.read_long(hunk)
            segments.append(hunk + 8)
            hunk = await self.read_long(hunk + 4)
            if hunk == 0:
                break
        return segments

    async def prologue(self):
        lines = await self.recv()
        data = {}
        # Memwatch 2: break at 00C7FDC0.W  W  0000000F PC=00C7BFEC CPUDW (000)
        if lines[0].startswith('Memwatch'):
            line = lines.pop(0)
            data['watch'] = int(line.split()[4].split('.')[0], 16)
        # Breakpoint at 00C04EB0
        if lines[0].startswith('Breakpoint'):
            line = lines.pop(0)
            data['break'] = int(line.split()[2], 16)
        # Exception 27, PC=00C15AC6
        while lines[0].startswith('Exception'):
            line = lines.pop(0)
            if not line.endswith('breakpoint'):
                data['exception'] = int(line[10:].split(',')[0])
        # just processor state
        data['regs'] = ParseProcessorState(lines)
        return data

    async def kill(self):
        # {q} Quit the emulator.
        self.send('q')


async def UaeDebugger(uaedbg):
    # Call FS-UAE debugger on CTRL+C
    loop = asyncio.get_event_loop()
    loop.add_signal_handler(signal.SIGINT, uaedbg.interrupt)

    history = InMemoryHistory()
    session = PromptSession('(debug) ', history=history)
    with patch_stdout():
        try:
            lines = await uaedbg.recv()
            while lines is not None:
                for line in lines:
                    print(line)
                try:
                    cmd = ''
                    while not cmd:
                        cmd = await session.prompt_async()
                        cmd.strip()
                        # prompt_async removes our SIGINT handler :(
                        loop.add_signal_handler(signal.SIGINT,
                                                uaedbg.interrupt)
                    uaedbg.send(cmd)
                except EOFError:
                    uaedbg.resume()
                except KeyboardInterrupt:
                    uaedbg.kill()
                lines = await uaedbg.recv()
        except asyncio.CancelledError:
            pass
        except EOFError:
            pass
        except Exception as ex:
            print('Debugger bug!')
    print('Quitting...')


class UaeProcess(UaeCommandsMixin):

    def __init__(self, proc):
        self.proc = proc
        self.watchpoints = {}

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
        logging.debug(f"(uae) <- " + repr(cmd))
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
                lines = [line.rstrip() for line in text.splitlines()]
                for line in lines:
                    logging.debug("(uae) -> " + repr(line))
                return lines

    async def logger(self):
        try:
            while not self.proc.stdout.at_eof():
                raw_text = await self.proc.stdout.readline()
                print(raw_text.decode(), end='')
        except asyncio.CancelledError:
            pass
        except asyncio.IncompleteReadError:
            pass
