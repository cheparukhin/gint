gint project
============

gint (pronounce 'guin') is a low-level library for fx-9860G calculators. It
provides the tools needed to develop programs under Linux using the gcc
toolchain (sh3eb-elf).

By the way, gint is free software; you may use it for any purpose, share it,
modify it and share you changes. No credit of any kind is needed, though
appreciated.



Interrupt handler
-----------------

The interrupt handler is the lowest-level part of the library. It directly
accesses the peripheral modules and performs keyboard analyzes, swaps screen
buffers, etc.

gint does not allow user programs to use their own handlers. However, it is
possible to map interrupt-driven events to user callbacks using the public API
(which is not possible with the system's interrupt handler). This may be
particularly useful for timers and RTC (the 16 Hz interrupt can be used as a
basis for a physical engine).



Public Interface
----------------

gint's API provides access to keyboard, timers, clock and more. It does some
powerful drawing and offers reliable multi-getkey, a gray engine, facilitates
register access and implements a few standard functions.



Building and installing
-----------------------

There a some dependencies:
* The `sh3eb-elf` toolchain somewhere in the PATH
* The fxSDK installed and available in the PATH

The easiest way to build gint is simply to enter a terminal and execute `make`.
This will build the following components :
* `libgint.a`, the gint library
* `libc.a`, a (very) few standard procedures
* `gintdemo.g1a`, a test application

The common `clean`, `mrproper`, and `distclean` rules will clean the directory.



Source organization
-------------------

gint is made of *modules*. Each module may have any of the following
components:
* A header file in `/include`
* An internal header file in `/include/internals`
* Single-function source files in `/src/module`: to avoid linking against the
  whole library, some functions have their own object files. Their names are
  those of the functions.
* Other source files in `/src/module`: contain multiple functions that always
  work together, or are lightweight enough not to be separated. Their names
  often begin with `module_`.
* Other files in `/src/module`: the `display` module contains a font, I think.

The demo application is in the `demo` directory.

The `doc` folder contains some documentation.
