#!/usr/bin/env python3

import argparse
import os
import os.path
import shutil
import shlex
import subprocess
from libtmux import Server, Session, exc


def HerePath(*components):
    return os.path.join(os.getenv('TOPDIR', ''), *components)


SOCKET = 'fsuae'
SESSION = 'fsuae'
GDBSERVER = 'uaedbg.py'
REMOTE = 'localhost:8888'
TMUX_CONF = HerePath('.tmux.conf')


class Launchable():
    def __init__(self, name, cmd):
        self.name = name
        self.cmd = cmd
        self.window = None
        self.options = []

    def configure(self, *args, **kwargs):
        raise NotImplementedError

    def start(self, session):
        cmd = ' '.join([self.cmd] + list(map(shlex.quote, self.options)))
        # XXX: print `cmd` to display command and its arguments
        self.window = session.new_window(
            attach=False, window_name=self.name, window_shell=cmd)


class FSUAE(Launchable):
    def __init__(self):
        super().__init__('fs-uae', HerePath('tools', GDBSERVER))

    def configure(self, floppy=None, rom=None, debug=False):
        self.options.extend(['-e', 'fs-uae'])
        if debug:
            self.options.append('-g')
        # Now options for FS-UAE.
        self.options.append('--')
        if floppy:
            self.options.append('--floppy_drive_0=' + os.path.realpath(floppy))
        if rom:
            self.options.append('--kickstart_file=' + os.path.realpath(rom))
        if debug:
            self.options.append('--use_debugger=1')
        self.options.append(HerePath('effects', 'Config.fs-uae'))


class SOCAT(Launchable):
    def __init__(self, name):
        super().__init__(name, 'socat')

    def configure(self, tcp_port):
        # The simulator will only open the server after some time has
        # passed.  To minimize the delay, keep reconnecting until success.
        self.options = [
            'STDIO', 'tcp:localhost:%d,retry,forever,interval=0.01' % tcp_port]


class GDB(Launchable):
    def __init__(self):
        super().__init__('gdb', 'm68k-amigaos-gdb')
        # gdbtui & cgdb output is garbled if there is no delay
        self.cmd = 'sleep 0.5 && ' + self.cmd

    def configure(self, program=''):
        self.options += ['-ex=set prompt \033[35;1m(gdb) \033[0m']
        self.options += [
            '-iex=directory {}/'.format(HerePath()),
            '-ix={}'.format(HerePath('.gdbinit')),
            '-ex=set tcp connect-timeout 30',
            '-ex=target remote {}'.format(REMOTE),
            '--nh',
            '--silent',
            program]


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Launch effect in FS-UAE emulator.')
    parser.add_argument('-r', '--rom', metavar='ROM', type=str,
                        help='Replace Amiga Kickstart with provided ROM.')
    parser.add_argument('-f', '--floppy', metavar='ADF', type=str,
                        help='Floppy disk image in ADF format.')
    parser.add_argument('-e', '--executable', metavar='EXE', type=str,
                        help='Provide executable file for GDB debugger.')
    parser.add_argument('-d', '--debug', choices=['gdbserver', 'gdb'],
                        help=('Run gdbserver on {} and launch gdb '
                              'if requested.'.format(REMOTE)))
    parser.add_argument('-w', '--window', metavar='WIN', type=str,
                        default='fs-uae',
                        help='Select tmux window name to switch to.')
    args = parser.parse_args()

    # Check if floppy disk image file exists
    if args.floppy and not os.path.isfile(args.floppy):
        raise SystemExit('%s: file does not exist!' % args.floppy)

    # Check if rom file exists
    if args.rom and not os.path.isfile(args.rom):
        raise SystemExit('%s: file does not exist!' % args.rom)

    # Check if executable file exists.
    if args.debug and not os.path.isfile(args.executable):
        raise SystemExit('%s: file does not exist!' % args.executable)

    uae = FSUAE()
    uae.configure(floppy=args.floppy, rom=args.rom, debug=args.debug)

    ser_port = SOCAT('serial')
    ser_port.configure(tcp_port=8000)

    par_port = SOCAT('parallel')
    par_port.configure(tcp_port=8001)

    if args.debug == 'gdb':
        debugger = GDB()
        debugger.configure(args.executable)

    subprocess.run(['tmux', '-f', TMUX_CONF, '-L', SOCKET, 'start-server'])

    server = Server(config_file=TMUX_CONF, socket_name=SOCKET)

    if server.has_session(SESSION):
        server.kill_session(SESSION)

    session = server.new_session(session_name=SESSION, attach=False,
                                 window_name=':0', window_command='sleep 1')

    try:
        uae.start(session)
        ser_port.start(session)
        par_port.start(session)
        if args.debug == 'gdb':
            debugger.start(session)

        session.kill_window(':0')
        if args.debug == 'gdb':
            session.select_window(debugger.name)
        else:
            session.select_window(args.window or par_port.name)
        Session.attach(session)
    except exc.TmuxObjectDoesNotExist:
        pass
    finally:
        Server.kill(server)
