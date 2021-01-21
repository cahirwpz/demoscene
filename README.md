Ghostown & Whelpz 2021
===
> "Perfection is achieved, not when there is nothing left to add, but when
> there is nothing left to take away." - Antoine de Saint-Exupery.

> "Everything should be as simple as possible, but no simpler." - Albert Einstein

\
How to run and debug effects?
===

This tutorial should work on *macOS* and *Linux*. It may be possible to
reproduce all steps also on *Windows*, but it will be likely an order of 
magnitude more difficult. I assume you have some former experience with 
Unix-like command line. 

**IMPORTANT!** Each time I push a commit to the repository a preconfigured
virtual machine is started and builds libraries and effects. The repository
contents is guaranteed to sucessfully build in well defined environment that is
provided by VM image prepared with [Docker](https://www.docker.com/)! If you
have some problems on your local machine **it's yours responsibility** to fix
it, unless following icon does say that build failed:
[![Build status](https://circleci.com/gh/cahirwpz/demoscene.png)](https://circleci.com/gh/cahirwpz/demoscene)

Setting up build environment
---

You need to reproduce the build environment I mentioned above. Fortunately
_Dockerfiles_ list all commands required to do so, at least on _Debian 9_ for
_x86-64_ architecture. This is a good starting point for most of you.
Please only consider lines starting with _ADD_ and _RUN_ – please refer to
[Dockerfile](https://docs.docker.com/engine/reference/builder/#run)
documentation if needed.

1. Start with [demoscene-toolchain](https://github.com/cahirwpz/demoscene-toolchain/blob/master/Dockerfile)
   _Dockerfile_. When the toolchain is built you must set up your `PATH`
   environment variable correctly. If you installed the toolchain in
   `${HOME}/amiga` then add `export PATH=${PATH}:${HOME}/amiga/bin` to your
   favorite shell initialization file (`.bashrc`, `.zshrc`, etc.). To validate
   your setup, please issue `which m68k-amigaos-gcc` command which should print
   where the compiler has been installed.

2. Follow up with [demoscene](https://github.com/cahirwpz/demoscene/blob/master/Dockerfile)
   _Dockerfile_. If you haven't had [Git LFS](https://git-lfs.github.com/)
   installed previously, you'll have to issue `git lfs install` command. Then
   clone *demoscene* repository once again to pull in binary files, otherwise
   you'll get links instead of real data – which you can fix by issuing
   `git lfs pull` command.

The Amiga emulator
---

You need the emulator to test Amiga binaries. Luckily, [demoscene-toolchain](https://github.com/cahirwpz/demoscene-toolchain) 
contains [fs-uae](https://fs-uae.net) emulator which is patched to enable 
diagnostic messages redirection from Amiga parallel port to Unix terminal. 
The emulator executable is installed in the same direcotry as other 
toolchain's executables and since  this directory is added to your `PATH` environment 
variable you should be able to use it without troubles. To verify that, please issue 
`which fs-uae` command and confirm that it prints correct installation path.

Compiling source code
---

Navigate to the [demoscene](https://github.com/cahirwpz/demoscene) repository 
you've just cloned and issue `make` command. If you have any problem with compiling, 
please make sure you have performed all steps listed above correctly. If `make` happens 
to complain about missing command – find the software package and install it.

Running the effect under emulator
---

At first, set the correct path to a kickstart file in 
[Config.fs-uae](https://github.com/cahirwpz/demoscene/blob/master/effects/Config.fs-uae) 
by adding _kickstart_file_ option at the end of this emulator configuration file. 
For example:

_kickstart_file = ~/my_amiga_roms/kick13.A500.rom_

After that, navigate to any effect's directory and issue `make run` command. 
**run** _make target_ prepares all files in `data` directory, builds 
executable file, creates ADF floppy image from binary files, adds 
custom bootloader to ADF and runs the _launcher_ tool, which in turn spawns 
the _fs-uae_ emulator, extends *UAE built-in debugger* (press F10 key to trigger it), 
and redirects messages from Amiga parallel port to Unix terminal.

Debugging the effect under emulator
---

Sometimes you need to tune your effect or fix some bugs. Depending on the complexity, 
you can use either console logging or low level *UAE built-in debugger*. But for your 
utmost convenience, in addition to these fantastic debugging approaches, this 
_framework_ comes with the support of a **native AmigaHunk executable debugger**!

As the debugger is based on well known *GDB: The GNU Project Debugger* it does the 
job very well but deserves a little more user friendly interface. 
[gdb-dashboard](https://github.com/cyrus-and/gdb-dashboard) should meet these expectations. 
Being in root folder of cloned repository, issue a command 
`wget -O gdb-dashboard https://git.io/.gdbinit` and you are done. You are able to 
quite comfortably debug your effects in the Unix terminal.

Now, navigate to any effect's directory and issue `make debug` command. **debug** 
_make target_ behaves the same way as **run** _make target_. It prepares all the 
prerequisites and launches the effect in the emulator. But, in addition, it runs 
a _gdb_ process, so all you need is to know how to talk with it. After some 
playing with _gdb_ breakpoint and stepping commands you should see a similarly 
looking terminal view. It looks massive! Isn't it?

![gdb-dashboard](./README.gdb.png)

Setting up Visual Studio Code IDE
---

The terminal based approach is productive when you are low-level geek - 
but what, when unfortunately you're not?

In such a case the provided Visual Studio Code integration should help. 
Yes, **you can use modern IDE with all its advantages for Amiga demoscene development!**

First of all, you have to install [Visual Studio Code](https://code.visualstudio.com/download) 
for your favorite platform. Then, using its extension explorer install following extensions:
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) (required)
- [Python](https://marketplace.visualstudio.com/items?itemName=ms-python.python)
- [m68k](https://marketplace.visualstudio.com/items?itemName=steventattersall.m68k)
- [Better C++ Syntax](https://marketplace.visualstudio.com/items?itemName=jeff-hykin.better-cpp-syntax)
- [GNU Linker Map files](https://marketplace.visualstudio.com/items?itemName=trond-snekvik.gnu-mapfiles)
- [Git History](https://marketplace.visualstudio.com/items?itemName=donjayamanne.githistory)
- [File Utils](https://marketplace.visualstudio.com/items?itemName=sleistner.vscode-fileutils)

The first one is required, but others are recommended for your convenience.

Having the installation done, navigate to the root folder of cloned _demoscene_ 
repository, type `code .` and _Visual Studio Code IDE_ will open the current 
directory as a _workspace_. Go to the `.vscode` folder and open `settings.json` 
in the editor view. Among other settings you can see `"effect": "anim"` setting which 
tells the _IDE_ that all operations like _clean/build/run/debug_ have to be 
performed on the _anim_ effect. You can change it to any other effect located in `effects` 
folder or use a special variable `"effect": "${fileDirnameBasename}"` which means 
that current opened file's folder name will be used to identify the effect for 
all operations mentioned above.

Now, open the `.vscode/keybindings.json` and copy its contents. Press `Shift+Cmd+P` 
and start typing `Open Keyboard Shortcuts (JSON)`. When this option appears on the 
list - choose it. It will open user's key bindings configuration file where you have 
to paste already copied contents.

Since now, you can use following keyboard shortcuts:
- `Shift+Cmd+C` - Clean output files of single effect
- `Shift+Cmd+B` - Build single effect
- `Shift+Cmd+R` - Run the effect without debugging
- `Shift+Cmd+D` - Start debugging the effect
- `Shift+Cmd+A, Shift+Cmd+C` - Clean all output files
- `Shift+Cmd+A, Shift+Cmd+B` - Build all effects

> These shortcuts are _macOS_ specific. For other operating systems please 
refer to _Visual Studio Code_ documentation for information which key to 
use instead of `Cmd`.

All these actions are handled by _Tasks_ defined in `.vscode/tasks.json`. You can 
also press `Shift+Cmd+T` or use `Terminal->Run Task...` menu item to call 
any of the above actions.

Let's check how it works. Open the `effects/anim/anim.c` source code file 
and set the breakpoint somewhere in the `Render` function. Press `Shift+Cmd+D` 
and... enjoy how modern _Visual Studio Code IDE_ is supporting 
Amiga demoscene development!

![vscode](./README.vsc.png)

*If you managed to get through the steps sucessfully, congratulations! You have
probably the most sophisticated cross development environment for Amiga 500 at
your disposal!*
---

\
Amiga 500 knowledge base
===

### Documentation

1. [Amiga Hardware Reference Manual](http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node0000.html)

2. [MC68000 instruction timing](http://linas.org/mirrors/www.nvg.ntnu.no/2002.09.16/amiga/MC680x0_Sections/mc68000timing.HTML)

3. [Custom Register Index](http://www.winnicki.net/amiga/memmap/)

4. [MOTOROLA M68000 FAMILY Programmer's Reference Manual](http://www.freescale.com/files/archives/doc/ref_manual/M68000PRM.pdf)

5. [DMA Time Slot Allocation / Horizontal Line](http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node02D4.gif)

### Tricks

1. English Amiga Board - coders forum:
   * [Asm / Hardware](http://eab.abime.net/forumdisplay.php?f=112)
   * [Undocumented Amiga hardware stuff](http://eab.abime.net/showthread.php?t=19676)

2. Amiga Demoscene Archive Forum: [coding](http://ada.untergrund.net/?p=boardforums&forum=4)
   * [Atari style C2P](http://ada.untergrund.net/?p=boardthread&id=217)

3. Subpixel line drawing using Blitter: [part 1](http://scalibq.wordpress.com/2011/12/28/just-keeping-it-real-part-5/) and [part 2](http://scalibq.wordpress.com/2012/01/06/just-keeping-it-real-part-5-1/).
