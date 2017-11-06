How to build and run effect?
===

This tutorial should work on *MacOSX* and *Linux*. It may be possible to
reproduce all steps on *Windows*, but it will be likely an order of magnitude
more difficult. I assume you have some former experience with Unix-like command
line. 

**IMPORTANT!** Each time I push a commit to the repository a preconfigured
virtual machine is started and builds libraries and effects. The repository
contents is guaranteed to sucessfully build in well defined environment that is
provided by VM image prepared with [Docker](https://www.docker.com/)! If you
have some problems on your local machine **it's yours responsibility** to fix
it, unless following icon does say that build failed:
[![Build status](https://circleci.com/gh/cahirwpz/demoscene.png)](https://circleci.com/gh/cahirwpz/demoscene)

Set up build environment
---

You need to reproduce the build environment I mentioned above. Fortunately
_Dockerfiles_ list all commands required to do so, at least on _Debian 9_ for
_x86-64_ architecture. This is a good starting point for most of you.
Please only consider lines starting with _ADD_ and _RUN_ – please refer to
[Dockerfile](https://docs.docker.com/engine/reference/builder/#run)
documentation if needed.

1. Start with [amigaos-cross-toolchain](https://github.com/cahirwpz/amigaos-cross-toolchain/blob/master/Dockerfile)
   _Dockerfile_. When the toolchain is built you must set up our `PATH`
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

Set up the emulator
---

Finally you need to setup the emulator to test Amiga binaries. If you've already
installed binary distribution of [fs-uae](https://fs-uae.net/download-devel)
development version that's ok. However **if you're thinking about debugging
seriously**, you need to enable diagnostic messages redirection from Amiga
parallel port to unix terminal. Unfortunately that cannot be done easily without
fixing the emulator. Please go to _fs-uae_ repository branch
[debug-fix](https://github.com/cahirwpz/fs-uae/tree/debug-fix),
download and [compile](https://fs-uae.net/compiling) it. You'll save yourself
some troubles if the directory with _fs-uae_ binary is in the `PATH` after
installation.

Compiling source code
---

After you checked out the repository, just issue `make` command in `a500`
directory. If you have any problem with compiling, please make sure you have
performed all steps listed above correctly. If *make* happens to complain about
missing command – find the software package and install it.

Run the effect under emulator
---

Apply following modifications to emulator configuration and launcher script:
 - set path to kickstarts directory in [Config.fs-uae](https://github.com/cahirwpz/demoscene/blob/master/a500/effects/Config.fs-uae#L14)
 - make sure _fs-uae_ is already in `PATH` and `which fs-uae` prints
   installation path.

Go into effect directory and issue `make run` command.  **run** _make target_
prepares all files in `data` directory, builds executable file, creates 
ADF floppy image from binary files, adds custom bootloader to ADF and runs the
launcher. `RunInUAE` tool performs several functions – it spawns _fs-uae_,
extends *UAE built-in debugger* (press F10 key to trigger it), and redirects
messages from Amiga parallel port to unix terminal.

**If you managed to get through the steps sucessfully, congratulations! You have
probably the most sophisticated cross development environment for Amiga 500 at
your disposal!**
