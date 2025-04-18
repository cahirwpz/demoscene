#!/usr/bin/env python3

import argparse
import asyncio
import logging
import signal
import sys
import traceback

from debug.uae import UaeDebugger, UaeProcess
from debug.gdb import GdbConnection, GdbStub


BREAK = 0xCF47  # no-op: 'exg.l d7,d7'


async def UaeLaunch(loop, args):
    log_file = open(args.log, 'w') if args.log else None

    # Create the subprocess, redirect the standard I/O to respective pipes
    uaeproc = UaeProcess(
        await asyncio.create_subprocess_exec(
            args.emulator, *args.params,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE),
        log_file)

    gdbserver = None

    async def GdbClient(reader, writer):
        try:
            await GdbStub(GdbConnection(reader, writer), uaeproc).run()
        except Exception:
            traceback.print_exc()
            input('Press enter to quit...')
            # TODO: Better handling of gdbserver client disconnection
            #       Now it simply throws ValueError(data) exception
            #       in gdb.py:31 [recv_ack]
            uaeproc.terminate()

    async def GdbListen():
        await uaeproc.prologue()
        uaeproc.break_opcode('{:04x}'.format(BREAK))
        print('Listening for gdb connection at localhost:8888')
        gdbserver = await asyncio.start_server(
            GdbClient, host='127.0.0.1', port=8888)

    # Terminate FS-UAE when connection with terminal is broken
    loop.add_signal_handler(signal.SIGHUP, uaeproc.terminate)

    logger_task = asyncio.ensure_future(uaeproc.logger())

    if args.gdbserver:
        await GdbListen()
    else:
        prompt_task = asyncio.ensure_future(UaeDebugger(uaeproc))

    await uaeproc.wait()

    if gdbserver:
        gdbserver.close()


if __name__ == '__main__':
    # XXX: change from INFO to DEBUG to display messages exchanged between
    # fs-uae and code in `debug/uae.py`, gdb and code in `debug/gdb.py`
    logging.basicConfig(level=logging.INFO,
                        format='%(levelname)s: %(message)s')
    # logging.getLogger('asyncio').setLevel(logging.DEBUG)

    if sys.platform == 'win32':
        loop = asyncio.ProactorEventLoop()
    else:
        loop = asyncio.SelectorEventLoop()
    asyncio.set_event_loop(loop)
    # loop.set_debug(True)

    parser = argparse.ArgumentParser(
        description='Run FS-UAE with enabled console debugger.')
    parser.add_argument('-e', '--emulator', type=str, default='fs-uae',
                        help='Path to FS-UAE emulator binary.')
    parser.add_argument('-g', '--gdbserver', action='store_true',
                        help='Configure and run gdbserver on localhost:8888')
    parser.add_argument('-l', '--log', type=str, default=None,
                        help='Log stdout to a given file.')
    parser.add_argument('params', nargs='*', type=str,
                        help='Parameters passed to FS-UAE emulator.')
    args = parser.parse_args()

    uae = UaeLaunch(loop, args)
    loop.run_until_complete(uae)
    loop.close()
