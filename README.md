# gint project

gint (pronounce “guin”) is an add-in unikernel for CASIO calculators of the
fx-9860G II and fx-CG 50 families. It provides a mostly free-standing runtime
and is used to develop add-ins under Linux, along with specialized GCC
toolchains and the [fxSDK](/Lephenixnoir/fxsdk).

When running in an add-in, gint takes control of the calculator's hardware
from the operating system, and manages it with its own drivers. It exposes a
new, richer API that takes advantage of the full capabilities of the machine.

This is free software: you may use it for any purpose, share it, modify it, and
share your changes. Credit is not required, but please let me know!

gint also includes third-party code that is distributed under its own license.
Currently, this only includes:

* A stripped-down version of the [TinyMT random number generator](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/TINYMT/index.html)
  ([GitHub repository](https://github.com/MersenneTwister-Lab/TinyMT)) by
  Mutsuo Saito and Makoto Matsumoto. See `src/3rdparty/tinymt32/LICENSE.txt`.

## Programming interface

Because of its free-standing design, gint's API provides direct and efficient
access to low-level MPU features, which includes:

* Multi-key management with event systems suitable for games
* Hardware timers with sub-millisecond and sub-microsecond resolution
* Fast screen drivers with DMAC on fx-CG 50
* Efficient and user-extendable interrupt management
* Safe access to on-chip and DSP memory areas
* Hardware-driven memory primitives (DMA, DSP)

The library also offers powerful higher-level features:

* An enhanced version of the system's GetKey() and GetKeyWait()
* A gray engine that works by rapidly swapping monochrome images on fx-9860G II
* Blazingly fast rendering functions (image rendering is 10 times faster tha
  MonochromeLib)
* Integrated font management

A couple of libraries extend these features, including:

* [libprof](/Lephenixnoir/libprof): Profiling and performance evaluation
* [libimg](/Lephenixnoir/libimg): Versatile image transformations
* [OpenLibm](/Lephenixnoir/OpenLibm): A port of the standard math library
* Integration with [a Newlib port by Memallox](/PlaneteCasio/libc) (unstable)

## Installing with GiteaPC

gint can be installed automatically with [GiteaPC](/Lephenixnoir/GiteaPC).

```bash
% giteapc install Lephenixnoir/gint
```

Normally you don't use gint directly, instead the fxSDK provides project
templates that are set up to use gint. Please see the
[fxSDK README file](/Lephenixnoir/fxsdk) for details.

## Building and installing manually

gint is built using the [fxSDK](/Lephenixnoir/fxsdk), which provides a suitable
CMake environment for the calculator. gint is always installed in the
compiler's install path (as given by `sh-elf-gcc --print-search-dirs`) which is
detected automatically, so normally you don't need to set the install prefix.

fx-CG 50 developers probably want a g3a wrapper as well; the reference
implementation is Tari's [mkg3a](https://gitlab.com/taricorp/mkg3a). This is
needed at the very last compilation step to create the g3a file. On Arch Linux,
you can use the [AUR/mkg3a](https://aur.archlinux.org/packages/mkg3a) package
maintained directly by Tari.

**Building for fx-9860G II**

`fxsdk build-fx` will invoke CMake and make. If you have specific configuration
options, run once with `-c` to configure.

```
% fxsdk build-fx -c <OPTIONS...>
```

Run without `-c` to build. This configures automatically.

```
% fxsdk build-fx
```

The available options are:

* `-DGINT_STATIC_GRAY=1`: Put the gray engine's VRAMs in static RAM instead of
  using `malloc()`

**Building for fx-CG 50**

Same as fx-9860G II, except the command is `fxsdk build-cg` instead of `fxsdk
build-fx`.

The available options are:

* `-DGINT_USER_VRAM=1`: Store all VRAMs in the user stack (takes up 350k/512k)

## Using in CMake-based add-ins

Find the `Gint` module and link against `Gint::Gint`.

```cmake
find_module(Gint 2.1 REQUIRED)
target_link_libraries(<target_name> Gint::Gint)
```

## Using in Makefile-based add-ins

Projects created with the fxSDK link with gint out-of-the-box. If you're not
using the fxSDK, you will need to:

* Build with `-ffreestanding -fstrict-volatile-bitfields`;
* Link with `-T fx9860g.ld` and `-lgint-fx` on fx-9860G;
* Link with `-T fxcg50.ld` and `-lgint-cg` on fx-CG 50.

If you don't have a standard library such as
[Memallox's port of newlib](/PlaneteCasio/libc), you also need `-nostdlib`. I
typically use `-m3 -mb` or `-m4-nofpu -mb` to specify the platform, but that
may not even be necessary.
