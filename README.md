gint project
============

gint (pronounce 'guin') is a low-level library for fx-9860G calculators. It
provides a mostly free-standing runtime support for the platform, and can be
used to develop add-ins under Linux along with the gcc toolchain (`sh3eb-elf`
target) and the [fxSDK](http://git.planet-casio.com/lephe/fxsdk).

gint is free software: you may use it for any purpose, share it, modify it and
share your changes. No credit of any kind is required, though appreciated.


Programming interface
---------------------

Because of its free-standing design, gint's API provides direct and efficient
access to the low-level MPU features, among which:
* Extensive keyboard control, including replication of the system behavior for
  office applications and event-driven decisions for games
* Hardware timers running at over 10 MHz, allowing microsecond-level control
* Unlimited mappings of callbacks to Real-Time Clock events (requires build)
* Access to processor register for debugging information, determination of
  processor speed (overclock is on the TODO list), backlight management...

The library also offers powerful higher-level features:
* A gray engine that works by rapidly swapping monochrome images
* Blazingly fast drawing functions when working with the fxSDK
* C Standard functions such as the `printf()` family.


Interrupt handler
-----------------

The interrupt handler is the lowest-level part of the library. It directly
accesses the peripheral modules and workarounds the system to perform keyboard
analyzes directly on hardware or timer management.

gint does not allow user programs to use their own handlers, but it allows them
to complete the original handler if they want to use interrupts that are not
supported by the library. It is also possible to map various interrupt-driven
events to user-provided callbacks using the API, which is not allowed by the
operating system. This is particularly useful for timers and the Real-Time
Clock (the 16 Hz interrupt can be used to run a physical engine, while the 1 Hz
interrupt can be used in a real-time management game).


Building and installing
-----------------------

To build and install gint, you will need the following components:
* The `sh3eb-elf` toolchain linked somewhere in the PATH
* The [fxSDK](http://git.planet-casio.com/lephe/fxsdk) installed and available
  in the PATH

The classical way to build gint is to enter a terminal and use the usual:

	$ ./configure
	$ make
	# make install

This will build and install the following components in the storage folder of
the fxSDK:
* `libgint.a`, the gint library
* `libc.a`, the partial standard library
* The libgint headers for development

The following additional files will be generated in the working directory:
* `gintdemo.g1a`, a test application

The usual `clean`, `mrproper`, and `distclean` rules will clean the directory.
There are configuration options, which can be obtained using
`./configure --help`. Most of them customize size limits, if a project needs to
extend them. The true free-standing program may build the library using the
`--no-syscalls` switch, but some features will be disabled (dynamic
allocation...).


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
* Other files in `/src/module`: the `display` module contains a font.

The demo application is in the `demo` directory.
