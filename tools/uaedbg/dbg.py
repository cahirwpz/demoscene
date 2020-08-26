import asyncio
import logging
import os

from prompt_toolkit.patch_stdout import patch_stdout
from prompt_toolkit.shortcuts import PromptSession
from prompt_toolkit.history import InMemoryHistory

from .info import Symbol, SourceLine, DebugInfoReader
from .state import BreakPoint, Registers


def print_lines(lines):
    for line in lines:
        print(line)


class UaeDebugger():
    def __init__(self, uae, executable):
        self.uae = uae
        self.executable = executable
        self._debuginfo = None
        self.breakpoints = []
        self.registers = Registers()

    async def address_of(self, where):
        try:
            return int(where, 16)
        except ValueError:
            pass

        debuginfo = await self.debuginfo()
        if debuginfo:
            res = debuginfo.find_line_addr(where)
            if res:
                return res[1]
            res = debuginfo.find_symbol_addr(where)
            if res:
                return res[1]
        return None

    async def break_info(self, pc):
        debuginfo = await self.debuginfo()
        if debuginfo:
            try:
                return str(debuginfo.find_line(pc)[1])
            except KeyError:
                pass
        return '%08X' % pc

    def break_lookup(self, addr):
        for bp in self.breakpoints:
            if bp.address == addr:
                return addr
        return None

    async def break_show(self, pc):
        print_lines(await self.uae.disassemble(pc, 5))
        print()
        debuginfo = await self.debuginfo()
        if debuginfo:
            print('Stopped at %s:' % await self.break_info(pc))
            try:
                _, sl = debuginfo.find_line(pc)
                for n in range(sl.line - 2, sl.line + 3):
                    indicator = ' >'[n == sl.line]
                    try:
                        print('{} {:4d} {}'.format(indicator, n, sl.source[n]))
                    except IndexError:
                        pass
            except KeyError:
                pass

    async def prologue(self):
        data = await self.uae.prologue()
        if 'regs' in data:
            self.regs = data['regs']
            print(self.regs)
            print('')
        if 'exception' in data:
            print('Stopped by exception %d' % data['exception'])
        await self.break_show(self.regs['PC'])

    async def debuginfo(self):
        if not self._debuginfo:
            segments = await self.uae.fetch_segments()
            if not segments:
                return
            try:
                self._debuginfo = DebugInfoReader(self.executable, segments)
            except ValueError:
                pass
        return self._debuginfo

    async def do_step(self):
        self.uae.step()
        await self.prologue()

    async def do_cont(self):
        self.uae.resume()
        print('Continue...')
        await self.prologue()

    async def do_memory_read(self, addr, length):
        addr = await self.address_of(addr)
        print(await self.uae.read_memory(addr, length))

    async def do_break_insert(self, addr):
        addr = await self.address_of(addr)
        if self.break_lookup(addr):
            return
        if not await self.uae.insert_hwbreak(addr):
            return
        bp = BreakPoint(addr)
        self.breakpoints.append(bp)
        print('Added breakpoint #%d, %s' %
              (bp.number, await self.break_info(bp.address)))

    async def do_break_remove(self, addr):
        addr = await self.address_of(addr)
        bp = self.break_lookup(addr)
        if not bp:
            return
        self.breakpoints.remove(bp)
        await self.uae.remove_hwbreak(addr)
        print('Removed breakpoint #%d' % bp.number)

    async def do_break_show(self):
        for bp in sorted(self.breakpoints):
            print('#%d: %s' % (bp.number, await self.break_info(bp.address)))

    async def do_disassemble_range(self, addr, end):
        addr = await self.address_of(addr)
        end = await self.address_of(end)
        while addr < end:
            line, = await self.uae.disassemble(addr, 1)
            addr += line.next_address
            print(line)

    async def do_info_registers(self):
        print(await self.uae.read_registers())

    async def do_debuginfo_symbol(self, symbol):
        debuginfo = await self.debuginfo()
        if not debuginfo:
            return
        sec, addr = debuginfo.find_symbol_addr(symbol)
        if addr:
            print('Symbol "%s" at %08X.' % (symbol, addr))
        else:
            print('No symbol "%s" found!' % symbol)

    async def do_debuginfo_source_line(self, source, line):
        debuginfo = await self.debuginfo()
        if not debuginfo:
            return
        sec, addr = debuginfo.find_line_addr("%s:%d" % (source, line))
        if addr:
            print('Line "%s:%d" at %08X.' % (source, line, addr))
        else:
            print('Line "%s:%d" found!' % (source, line))

    async def do_where_am_I(self):
        await self.break_show(self.regs['PC'])

    commands = {
        'mr': lambda self, arg:
            self.do_memory_read(arg[0], int(arg[1])),
        'b': lambda self, arg:
            self.do_break_insert(arg[0]),
        'bd': lambda self, arg:
            self.do_break_remove(arg[0]),
        'bl': lambda self, arg:
            self.do_break_show(),
        'dr': lambda self, arg:
            self.do_disassemble_range(arg[0], arg[1]),
        't': lambda self, arg:
            self.do_step(),
        'g': lambda self, arg:
            self.do_cont(),
        'r': lambda self, arg:
            self.do_info_registers(),
        'Zy': lambda self, arg:
            self.do_debuginfo_symbol(arg[0]),
        'Zc': lambda self, arg:
            self.do_debuginfo_source_line(arg[0], int(arg[1])),
        '!': lambda self, arg:
            self.do_where_am_I()
    }

    commands_ignored = ['Z', 'Ze', 'Zs', 'Zl']

    async def do_command(self, cmd):
        fs = cmd.split()
        if not fs:
            return
        op, arg = fs[0], fs[1:]
        if op in self.commands:
            await self.commands[op](self, arg)
        elif op in self.commands_ignored:
            print('Command ignored...')
        else:
            print_lines(await self.uae.communicate(cmd))

    async def run(self):
        history = InMemoryHistory()
        session = PromptSession('(debug) ', history=history)
        with patch_stdout():
            try:
                await self.prologue()
                while True:
                    try:
                        cmd = await session.prompt_async()
                        await self.do_command(cmd.strip())
                    except EOFError:
                        await self.do_cont()
            except KeyboardInterrupt:
                self.uae.terminate()
            except asyncio.CancelledError:
                pass
            except EOFError:
                pass
            except Exception as ex:
                logging.exception('Debugger bug!')
        print('Quitting...')
