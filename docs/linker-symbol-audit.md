# Linker-symbol audit — 16 October 2026

Scope: the fork based on upstream `463f180`, all assignments in `fx9860g.ld.c`,
`fxcg50.ld.c`, and `fxcp_hh3.ld.c`, and their uses in C, headers, and assembly.
This is a targeted audit of the compiler/linker boundary, not an audit of every
driver or an assertion that gint has no other bugs.

## Confirmed failure

The CG linker defines `_lreloc` and `_sreloc` as zero. Startup declared them as
`uint32_t` objects, then interpreted their addresses as a table pointer and a
byte count. GCC 16.2 could infer that a real four-aligned object's address is
nonzero and therefore at least four. It compiled the relocation loop without
an entry check, using `((_sreloc - 4) >> 2) + 1` iterations. Linking the size to
zero produced 2^30 iterations beginning at address zero.

The existing platform fix restricts that stage to `GINT_OS_FX`. This audit also
hardens its numeric size handling on FX, where an empty table is legitimate.
The new test runner executes the actual `src/kernel/start.c`; the original
upstream file fails its empty-section CG case with a simulator bus error.

## Findings and changes

| Use | Finding | Treatment and validation |
|---|---|---|
| `start.c`: `sdata`, `sbss`, `silram`, `sxyram`, `sgmapped`, `sreloc`, `srom` | Absolute sizes can be zero; typed-object address assumptions are inappropriate. | Byte-array declarations and `gint_linker_value()`. Tests cover zero and populated sections, zero/one/three relocations, and FX SH3's zero-length preload. |
| `start.c`: load/run addresses | Linker addresses do not define C object bounds; optional sections can have zero addresses. | Materialize values before converting to typed pointers; only access memory when its size permits it. |
| `start.c`: constructor/destructor boundaries | Empty tables can have equal boundaries, including zero. Relational comparisons of separately declared C objects are inappropriate. | Compare materialized integer addresses and call entries only inside the interval. Tests cover empty, one, and two callbacks, including both boundaries at zero. |
| `drivers.h`: driver count | Subtraction of two separately declared array pointers relies on a synthetic C array relation. | Subtract materialized integer boundaries, divide by the actual target struct size, and preserve the `ptrdiff_t` result type. Tests cover zero, one, and two driver records. |
| `kernel.c`: exception/TLB handler boundaries, `euram`, CP VBR region | Assembly/linker boundary labels were declared as scalar objects. No second observed crash. | Byte-array declarations and numeric value materialization; CG and FX builds validate these paths at compile/link time. Hardware copying and CP VBR behavior remain outside the simulator fixture. |
| `world.c`, `overclock.c`: driver iteration | Use the driver's public count and real populated records. | Covered by the shared count change; no unrelated iteration changes. |
| `hhk3_start.c`: `__hhk_address_*` | These are allocated, writable assembly words used for syscall relocation, not absolute size symbols. | Retain the existing object declarations. |
| Fonts, rendering assembly entry labels, and literal values in `.S` files | Refer to actual stored assets/code or are consumed directly by the assembler, without the C absolute-size inference. | No speculative changes. |
| Linker section layouts | Startup copy/clear helpers require sizes divisible by 16; the relevant scripts pad those sections to 16 bytes. Relocation entries are four-byte words. | Retain layout and bounds; fixtures exercise 0/16/32-byte sections and guard words around every buffer. |

`gint_linker_value()` uses an empty read/write register constraint. It emits no
machine instruction but prevents GCC and LTO from carrying C-object assumptions
into the numeric linker value. It is not a memory barrier and does not replace
`volatile`, cache management, or MMU synchronization. Merely adding another C
null check can be optimized away; link-only flags cannot undo optimizations
already recorded in library objects.

## Validation and limits

The target matrix runs GCC 16.2 with `-Os` and `-O2`, with and without LTO, for
CG, FX SH3/SH4 selections, and CP startup semantics. It checks data copying,
BSS clearing, optional sections, relocations, constructor/destructor order,
valid/invalid loader metadata, and return from `main()` through the real
SuperH setjmp/longjmp implementation. Hardware detection, MMU queries, OS
calls, and on-chip memory clearing are mocked.

No additional hardware failure was established by this audit. The remaining
changes remove instances of the same assumption rather than claiming new
observed defects. A real calculator launch/game/exit/relaunch check remains
required after rebuilding an add-in with these broader changes.

References: [GNU ld on source references to linker symbols](https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html),
[GCC's null-address assumptions](https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html#index-fdelete-null-pointer-checks).
