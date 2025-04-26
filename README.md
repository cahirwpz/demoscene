# Ghostown & Whelpz 2025

> "Perfection is achieved, not when there is nothing left to add, but when
> there is nothing left to take away." - Antoine de Saint-Exupery.

> "Everything should be as simple as possible, but no simpler." - Albert Einstein

## How to run and debug effects?

This tutorial should work on *macOS* and *Linux*. It may be possible to
reproduce all steps also on *Windows*, but it will be likely an order of
magnitude more difficult. I assume you have some former experience with
Unix-like command line.

**IMPORTANT!** Each time I push a commit to the repository a preconfigured
virtual machine is started and builds libraries and effects. The repository
contents is guaranteed to successfully build in well defined environment that is
provided by VM image prepared with [Docker](https://www.docker.com/)! If you
have some problems on your local machine **it's yours responsibility** to fix
it, unless following icon does say that build failed:
![Build status](https://github.com/cahirwpz/demoscene/actions/workflows/default.yml/badge.svg)

## Setting up build environment

You need to reproduce the build environment I mentioned above. Fortunately
*Dockerfiles* list all commands required to do so, at least on *Debian 12
(Buster)* for *x86-64* architecture. This is a good starting point for most of
you.  Please only consider lines starting with `ADD` and `RUN` – please refer to
[Dockerfile](https://docs.docker.com/engine/reference/builder/#run)
documentation if needed.

1. Start with [demoscene-toolchain](https://github.com/cahirwpz/demoscene-toolchain/blob/master/Dockerfile)
   *Dockerfile*. When the toolchain is built you must set up your `PATH`
   environment variable correctly. If you installed the toolchain in
   `${HOME}/amiga` then add `export PATH=${PATH}:${HOME}/amiga/bin` to your
   favorite shell initialization file (`.bashrc`, `.zshrc`, etc.). To validate
   your setup, please issue `which m68k-amigaos-gcc` command which should print
   where the compiler has been installed.

2. Follow up with [demoscene](https://github.com/cahirwpz/demoscene/blob/master/Dockerfile)
   *Dockerfile*. If you haven't had [Git LFS](https://git-lfs.github.com/)
   installed previously, you'll have to issue `git lfs install` command. Then
   clone *demoscene* repository once again to pull in binary files, otherwise
   you'll get links instead of real data – which you can fix by issuing
   `git lfs pull` command.

## The Amiga emulator

You need an emulator to test Amiga binaries. Luckily, [demoscene-toolchain](https://github.com/cahirwpz/demoscene-toolchain)
provides [fs-uae](https://fs-uae.net) emulator. Now, **important** thing –
vanilla `fs-uae` doesn't provide features that are required for full experience.
The emulator has been
[patched](https://github.com/cahirwpz/demoscene-toolchain/tree/master/patches),
which enables Amiga programs to efficiently output diagnostic messages using
[UAE traps](https://github.com/cahirwpz/demoscene/blob/master/include/uae.h) and
provides various fixes for debugger integration. The emulator is installed in
the same directory as the compiler. Since this directory has already been added
to your `PATH` environment variable you should be able to use it without
troubles. To verify that, please issue `which fs-uae` command and confirm that
it prints correct installation path.

## Compiling source code

Navigate to the [demoscene](https://github.com/cahirwpz/demoscene) repository
you've just cloned and issue `make` command. If you build process fails, please
verify that you have performed all steps listed above correctly. If `make`
happens to complain about missing command – find the software package and
install it. You cannot expect us to help you set up your environment!

## Running the effect under emulator

At first, you have to provide [Kickstart ROMs](https://fs-uae.net/docs/kickstarts) for the emulator to operate. `fs-uae` will automatically find the correct kickstart ROMs for all Amiga models if you have copied the `.rom` files into its [kickstart-dir](https://fs-uae.net/docs/options/kickstarts-dir).

After that, navigate to any effect's directory and issue `make run` command.
**run** make target prepares all files in `data` directory, builds executable
file, creates ADF floppy image from binary files, adds custom bootloader to ADF
and runs the *launcher* tool, which in turn spawns the *fs-uae* emulator,
extends *UAE built-in debugger* (press F10 key to trigger it), and redirects
messages from Amiga parallel port to Unix terminal.

## Debugging the effect under emulator

Sometimes you need to tune your effect or fix some bugs. Depending on the
complexity, you can use either console logging or low level *UAE built-in
debugger*. But for your utmost convenience, in addition to those ancient
debugging techniques, the toolchain comes with **AmigaHunk executable
debugger**!

The debugger is nothing more than well known [GDB: The GNU Project
Debugger](https://sourceware.org/gdb/current/onlinedocs/gdb/). It's **a very
powerful tool** (believe me!) but the default interface is not the most user
friendly one! Too soothe the experience you should install great *GDB* overlay
[gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard). In root directory
of cloned repository issue a command `wget -O gdb-dashboard
https://git.io/.gdbinit` and you are done! Now, you can quite comfortably debug
your effects in a terminal.

Now, just navigate to a directory with given effect and issue `make debug`
command. **debug** make target behaves the same way as **run** make target.
It prepares all the prerequisites and launches the effect in the emulator. But,
in addition, it runs a *GDB* process, so all you need is to know how to talk
with it. After playing with some basic *GDB* commands like `breakpoint`, `step`,
`next` and `continue`, you should be able to see a terminal view similar to
given below. It looks massive! Isn't it?

![gdb-dashboard](./README.gdb.png)

## Setting up Visual Studio Code IDE

Terminal based approach is more productive when you are low-level geek. But what
if you're not *GDB* aficionado? Fortunately there's option for you as well.
Though we still encourage you to learn *GDB* – it will pay off eventually ;-)

In such a case the provided Visual Studio Code integration should help.
**Oh yes! You can use modern IDE with all its advantages for Amiga
development!**

First of all, you have to install [Visual Studio
Code](https://code.visualstudio.com/download) for your favorite platform. Then,
using its extension explorer install following extensions:

- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) (required)
- [Python](https://marketplace.visualstudio.com/items?itemName=ms-python.python)
- [m68k](https://marketplace.visualstudio.com/items?itemName=steventattersall.m68k)
- [Better C++ Syntax](https://marketplace.visualstudio.com/items?itemName=jeff-hykin.better-cpp-syntax)
- [GNU Linker Map files](https://marketplace.visualstudio.com/items?itemName=trond-snekvik.gnu-mapfiles)
- [Git History](https://marketplace.visualstudio.com/items?itemName=donjayamanne.githistory)
- [File Utils](https://marketplace.visualstudio.com/items?itemName=sleistner.vscode-fileutils)

The first one is required, but others are recommended for your convenience.

Having the installation done, navigate to root directory of cloned *demoscene*
repository, type `code .` and *Visual Studio Code IDE* will open the current
directory as a *workspace*. Go to the `.vscode` folder and open `settings.json`
in the editor view. Among other settings you can see `"effect": "anim"` setting
which tells the *IDE* that all operations like *clean/build/run/debug* have to
be performed on `anim` effect. You can change it to any other effect located
in `effects` folder or use a special variable `"effect":
"${fileDirnameBasename}"` which means that current opened file's folder name
will be used to identify the effect for all operations mentioned above.

Now, open the `.vscode/keybindings.json` and copy its contents. Press
`Shift+Cmd+P` and start typing `Open Keyboard Shortcuts (JSON)`. When this
option appears on the list - choose it. It will open user's key bindings
configuration file where you have to paste already copied contents.

Since now, you can use following keyboard shortcuts:

- `Shift+Cmd+C` - Clean output files of single effect
- `Shift+Cmd+B` - Build single effect
- `Shift+Cmd+R` - Run the effect without debugging
- `Shift+Cmd+D` - Start debugging the effect
- `Shift+Cmd+A, Shift+Cmd+C` - Clean all output files
- `Shift+Cmd+A, Shift+Cmd+B` - Build all effects

> These shortcuts are *macOS* specific. For other operating systems please
refer to *Visual Studio Code* documentation for information which key to
use instead of `Cmd`.

All these actions are handled by *Tasks* defined in `.vscode/tasks.json`. You
can also press `Shift+Cmd+T` or use `Terminal->Run Task...` menu item to call
any of the above actions.

Let's check how it works. Open the `effects/anim/anim.c` source code file
and set the breakpoint somewhere in the `Render` function. Press `Shift+Cmd+D`
and... enjoy how modern *Visual Studio Code IDE* is supporting
Amiga demoscene development!

![vscode](./README.vsc.png)

**If you managed to get through the steps successfully, congratulations! You
have probably the most sophisticated cross development environment for Amiga 500
at your disposal!**
