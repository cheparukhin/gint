#include <gint/config.h>
#if GINT_OS_CP

/*
Binary format for HollyHock-2. HH2 requires the following header layout:
  0x00 (12 bytes): Entry point
  0x0c  (4 bytes): Load address
  0x10 (variable): Metadata as NUL-terminated strings

Additionally, HH2 only loads up to 128 kB, so we need a second-stage loader to
handle larger programs. We'll generally want to load these to the VRAM backup,
i.e. not the default.

Since we need a stage-2 loader, we might as well (1) let HH2 do the short
stage-1 load to the end of the RAM, (2) use the stage-2 loader to load the rest
of the add-in into the VRAM backup, and (3) scrap the end-of-RAM area and use
it as buffer/heap.

The only subtlety is that the stage-2 loader must know the binary file's path
to load from the filesystem, and HollyHock doesn't (yet) give it to us. We
write the path in the final binary with the fxsdk/scripts/patch_hh2_filename.py
script from the fxSDK. This is fragile (i.e. not renaming-proof), but a start.
Since the header is variable-size, I leave the offset of stage-2 in the binary
at offset 0x06. I could avoid this by patching the ELF, not bin, but I'm lazy.
*/

/* 16-byte header */
.section .hh2.header, "ax"
.global _start_header
.align 2

_start_header:
	mov.l	1f, r0
	jmp	@r0
	nop
	/* Offset to hardcoded filename, used to patch binary after linking */
	.word	(.stage2_filename - _start_header)
	/* Address of stage-2 */
1:	.long	_stage2
	/* Stage-1 load address at end of RAM (specified in linker script) */
	.long	_start_header

/* Note: User-provided, variable-size app metadata is linked in-between the
   .hh2.header and the .hh2.stage2 sections */

/* Stage-2 loader */
.section .hh2.stage2, "ax"
.align 2

_stage2:
	/* No-op load */
	mov.l	1f, r0
	jmp	@r0
	nop
	nop

1:	.long	_start

.align 2
	.zero 2
.stage2_path:
	.ascii "\\fls0\\"
.stage2_filename:
	/* Recognizable pattern so the patching script can double-check that
	   it's overriding the correct location. */
	.long 1, 2, 3, 4, 5, 6, 7, 8

#endif /* GINT_OS_CP */
