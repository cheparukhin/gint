# gint project

gint (pronounce 'guin') is a development system for Casio fx-9860G II and
fx-CG 50 calculators. It provides a mostly free-standing runtime and is used
to develop add-ins under Linux, along with specialized GCC toolchains and the
[fxSDK](/Lephenixnoir/fxsdk).

gint is a modular kernel that implements its own drivers for the calculator's
hardware, overriding the operating system and its syscalls. It is a drop-in
replacement from fxlib, with which it is mostly incompatible. gint exposes a
new, richer API to manipulate the hardware and take advantage of the full
capabilities of the machine.

This is free software: you may use it for any purpose, share it, and modify it
as long as you share your changes. Credit is not required, but please let me
know!

## Programming interface

Because of its free-standing design, gint's API provides direct and efficient
access to the low-level MPU features, among which:

* Multi-key management with event systems suitable for games
* Hardware timers with sub-millisecond and sub-microsecond resolution
* Fast screen drivers with DMAC on fx-CG 50
* Efficient and user-extendable interrupt management

The library also offers powerful higher-level features:

* An enhanced version of the system's GetKey() and GetKeyWait()
* A gray engine that works by rapidly swapping monochrome images on fx-9860G II
* Blazingly fast drawing functions when working with the fxSDK (image rendering
  is 10 times faster than MonochromeLib)
* Integrated font management with the fxSDK
* Integration with a Newlib port by Memallox (WIP)

## Building and installing gint

You can choose to build gint for fx-9860G II (monochrome calculators, aka
Graph 85 family), fx-CG 50 (color calculators, aka Prizm or Graph 90 family),
or both. There are a few dependencies:

* A suitable GCC toolchain in the `PATH`. You can absolutely *not* build gint
  with your system compiler!
  * For fx-9860G II, `sh3eb-elf` is strongly advised
  * For fx-CG 50, `sh4eb-elf` (with `-m4-nofpu`) is slightly better but
    `sh3eb-elf` is completely fine
* The [fxSDK](/Lephenixnoir/fxsdk) installed and available in the PATH. You
  will need `fxconv` to build gint, and if you intend to develop add-ins for
  fx-9860G II, you probably want `fxg1a` as well. All these tools are built by
  default.

fx-CG 50 developers probably want a g3a wrapper as well; the reference
implementation is tari's [mkg3a](https://www.taricorp.net/projects/mkg3a/).
This is only necessary when creating g3a files, not to use gint.

The build process is detailed below for both platforms, the principle is the
same. You can build both targets at the same time by reading the two sections.

By default gint will be installed in the appropriate compiler folder, which
is `$PREFIX/` for libraries and linker scripts, and `$PREFIX/include/gint/` for
headers, where `PREFIX` is obtained by running
`${toolchain}-gcc --print-search-dirs` and reading the line that starts with
`install:`. You can change this with the `--prefix` configure option.

### Building for fx-9860G II

Create a build directory and configure in it:

    % mkdir build.fx && cd build.fx
    % ../configure --target=fx9860g

Then build the source and install the library files to the selected directory.
You might need root access if you selected a target directory owned by root
with `--prefix`, or if you built your compiler as root.

    % make
    % make install

### Building for fx-CG 50

Create a build directory and configure in it. The default toolchain is
`sh4eb-elf`, if you wish to build with `sh3eb-elf`, you need to add a
command-line option `--toolchain=sh3eb-elf`.

    % mkdir build.cg && cd build.cg
    % ../configure --target=fxcg50

Then build the source and install the library files to the selected directory.

    % make
    % make install

## Using gint

To use gint as your runtime environment, the bare minimum is:

* Build with `-ffreestanding`;
* Link with `-T fx9860g.ld` and `-lgint-fx` on fx-9860G;
* Link with `-T fxcg50.ld` and `-lgint-cg` on fx-CG 50.

If you don't have a standard library such as
[Memallox's port of newlib](/PlaneteCasio/libc), you also need `-nostdlib`. I
typically use `-m3 -mb` or `-m4-nofpu -mb` to specify the platform, but that
may not even be necessary.

Typically you might want to do this with the [fxSDK](/Lephenixnoir/fxsdk),
which hides most of the details and makes it easy to roll add-ins.
