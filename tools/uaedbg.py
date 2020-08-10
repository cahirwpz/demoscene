#!/usr/bin/env python3

import argparse
import asyncio
import logging
import signal
import sys

from uaedbg.uae import UaeProcess
from uaedbg.dbg import UaeDebugger


async def UaeLaunch(loop, args):
    # Create the subprocess, redirect the standard I/O to respective pipes
    uaeproc = UaeProcess(
            await asyncio.create_subprocess_exec(
                'fs-uae', *args,
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE))

    uaedbg = UaeDebugger(uaeproc)

    # Terminate FS-UAE when connection with terminal is broken
    loop.add_signal_handler(signal.SIGHUP, uaeproc.terminate)

    # Call FS-UAE debugger on CTRL+C
    loop.add_signal_handler(signal.SIGINT, uaeproc.interrupt)
    prompt_task = asyncio.ensure_future(uaedbg.run())
    logger_task = asyncio.ensure_future(uaeproc.logger())

    await uaeproc.wait()


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO,
                        format='%(levelname)s: %(message)s')
    # logging.getLogger('asyncio').setLevel(logging.DEBUG)

    if sys.platform == 'win32':
        loop = asyncio.ProactorEventLoop()
        asyncio.set_event_loop(loop)
    else:
        loop = asyncio.get_event_loop()
    # loop.set_debug(True)

    parser = argparse.ArgumentParser(
        description='Run FS-UAE with enabled console debugger.')
    parser.add_argument('params', nargs='*', type=str,
                        help='Parameters passed to FS-UAE emulator.')
    args = parser.parse_args()

    uae = UaeLaunch(loop, args.params)
    loop.run_until_complete(uae)
    loop.close()
