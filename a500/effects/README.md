A few easy steps to run almost any effect

1. Compiling sources.
Just use simple ‚make’ command being at ‚a500’ folder.
If You have any problem with compiling plz be sure Your path to compiler is correct.
Also You can export compiler’s path :). 
example: export PATH=$PATH:/Users/edi/amigaos-cross-toolchain/m68k-amigaos/bin/

2. Making amiga disk.
Before You run emulator, You have to make a disk and add needed files.
You can find special tool at folder ‚tools’ called ‚fsutil.py’.

Just type ‚python tools/fsutil.py -b bootloader.bin create disk.adf ’
if something is wrong plz be sure that You have installed Python and file ‚bootloader.bin’ is at ‚a500’ folder.

3. Copying files on disk.
Now when You have disk You can start copying files. Almost all sources do not have assets but some of them do not need these. Just use same script and add files on the end.

Beofre that plz be sure You have needed file at folder ‚effects/stripes/‚
You will need compiled ‚stripes.exe’ file.

Type ‚python tools/fsutil.py -b bootloader.bin create args.adf effects/stripes/stripes.exe’
Now You have disk with ‚stripes.exe’ file.

4.Running emulator.
- https://github.com/cahirwpz/demoscene/blob/master/a500/effects/Config.fs-uae#L14 - put correct path to kikstarts folder.
- https://github.com/cahirwpz/demoscene/blob/master/a500/effects/RunInUAE#L51 - put correct path to emulator. On mac it has to be direct file in package for example like this one ‚../fs-uae.app/Contents/MacOS/fs-uae’.

After that You can run script ‚./RunInUAE’.
If everything works fine all logs should be placed at terminal.
