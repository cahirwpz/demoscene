How to run almost any effect from this directory?
===

This tutorial should work on *MacOSX* and *Linux*. We assume that you'll install following packages:
 * Python 2.7

Set up build environment
---

You need to build cross compiler for *AmigaOS*. Visit [amigaos-cross-toolchain](https://github.com/cahirwpz/amigaos-cross-toolchain) site and follow the instructions.

When the toolchain is build you must set up our `PATH` environment variable correctly. If you installed the toolchain in `${HOME}/amiga` then add `export PATH=${PATH}:${HOME}/amiga/bin` to your favorite shell initialization file (`.bashrc`, `.zshrc`, etc.).

Compile source code
---

After you checked out the repository, just issue `make` command in `a500` folder. If you have any problem with compiling make be sure your `PATH` environment variable is set up correctly.

Prepare Amiga floppy disk image
---
Before you run [FS-UAE](https://fs-uae.net/) emulator, you have to create a floppy disk image and add needed files there.
The tool to aid this process is `tools/fsutil.py`. Before you start using it make sure bootblock code is available in `a500/bootloader.bin`.

To make sure everything works fine and dandy try to create an empty image:
```
python tools/fsutil.py -b bootloader.bin create args.adf
```

If above works we can start copying files to the image. Almost all executable files require additional assets (images and other data), but some of them do not – for instance `stripes.exe` in `effects/stripes` directory.

To prepare a disk with `stripes` effect on it run:
```
python tools/fsutil.py -b bootloader.bin create args.adf effects/stripes/stripes.exe
```

Run the effect under emulator
---

Apply following modifications to emulator confguration and launcher script:
 - set correct path to kickstarts directory in [Config.fs-uae](https://github.com/cahirwpz/demoscene/blob/master/a500/effects/Config.fs-uae#L14)
 - set correct path to the emulator in [RunInUAE](https://github.com/cahirwpz/demoscene/blob/master/a500/effects/RunInUAE#L51). On *MacOSX* it has to point to the executable file in *FS-UAE* application directory, namely `/Applications/fs-uae.app/Contents/MacOS/fs-uae`

If you did everything correctly run `./RunInUAE` command and enjoy `stripes` effect. Some useful diagnostic messages will be printed on terminal.
