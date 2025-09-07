# SashOS

My little operating system call Sasha

## Prerequisites

The project requires a Unix-like environment.

For Part 3, you need the following tools:

* `make`
* `nasm`
* `qemu-system-x86` for testing
* your preferred text editor (i use VSCode)
* your preferred hex editor (i use Ghex)

## Build instructions

* run `make`

## Running with qemu

* run `./run.sh`

## Debugging with gdb

* run `./debug.sh`'
* in new terminal run gdb
* in gdb run `source init.gdb`
* Then use stepi + enter

## Help

To release the keyboard and mouse: Press Ctrl + Alt + G.

You will see the mouse cursor move freely out of the QEMU window. The keyboard will now type into your host machine's terminal or other applications.

[FAT12 Explanation](https://www.sqlpassion.at/archive/2022/03/03/reading-files-from-a-fat12-partition/)