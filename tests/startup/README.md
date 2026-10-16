# GCC 16 startup regressions

Requirements: Python 3.9+, a GCC 16+ SuperH compiler with fxlibc installed, and
GDB's `sh-elf-run` simulator. Put them on PATH, or pass `--cc` and `--sim`.
Missing tools and older compilers are errors rather than skipped tests.

```sh
python3 tests/startup/run.py
```

The default run executes 96 cases using the actual `src/kernel/start.c` and
public headers. It covers `-Os`/`-O2`, LTO on/off, CG, FX (SH3 and SH4 selections),
and CP. The instruction set is SH4; this does not certify SH3 hardware execution.
`--quick` selects only `-Os` with LTO (24 cases). `--output build-startup-test`
retains the generated fixture and final ELF for inspection.

Each case constructs real absolute linker symbols and section boundaries, then
executes the generated SH instructions. It covers zero and nonzero section sizes,
empty and populated constructor/destructor and driver tables, guard words around
buffers, empty and populated FX relocations, invalid loader information, and
return through the real setjmp/longjmp implementation. CP checks that data and
on-chip initialization remain the external loader's responsibility.

The harness replaces hardware/OS boundaries only. It does not emulate MMU,
caches, device interrupts, display, or Casio firmware. Simulator exit codes
1–11 identify the failed checks in `harness.c`; a signal or timeout is also a
failure. The fixture's constructor functions are explicit linker roots because
LTO and section garbage collection otherwise discard symbols referenced only
by linker-script `LONG` expressions.

To verify that the suite catches the original regression:

```sh
mkdir -p build-baseline
git show 463f180ffb71ccdcef05c928ad19f2a480b14bc1:src/kernel/start.c > build-baseline/start.c
python3 tests/startup/run.py --quick --startup-source build-baseline/start.c
```

That negative control must fail; with GCC 16.2 the first empty-section CG case
stops with a bus error. It is a sensitivity check, not an expected failure in
the normal test suite.

Host compatibility tests (OS selection and LCD callbacks) run separately:

```sh
python3 -m unittest discover -s tests -p 'test_*.py' -v
```

GitHub CI runs these host tests. The GCC 16 target matrix currently requires
the configured cross toolchain and simulator; it must be run locally before
updating the pinned fork commit in a consumer.
