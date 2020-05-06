/*
**	gint:core:inth - Interrupt handlers
**	This file only contains the entry points because the gates are managed
**	by device drivers. Each driver will supply its own interrupt handler
**	blocks depending on its configuration.
*/

.global _gint_inth_7305

#ifdef FX9860G
.global _gint_inth_7705
#endif

.section .gint.blocks, "ax"
.align 4

/* Interrupt handlers

   The .gint.blocks section consists of blocks of 32 bytes intended for mapping
   into the VBR space (exception, TLB miss, and interrupt handlers). Each event
   gate is linearly associated with a block number:

     block_id = (event_code - 0x380) / 0x20

   This file provides entry points; drivers may provide their own interrupt
   handlers, and store them in the .gint.blocks section for consistency. They
   should be aware of the consequences of reordering the blocks into the VBR
   space:

   - It is possible to map MPU-specific blocks at runtime, to avoid checking
     the MPU each time an interrupt is handled;
   - However, the blocks can only rely on relative displacements or cross-
     references if their relative order is known and heeded for. A good example
     of this is the timer driver. Please be careful. */

/* SH7305-TYPE DEBUG INTERRUPT HANDLER - 26 BYTES */

#if 0
_gint_inth_7305:
	mov.l	1f, r0
	mov.l	@r0, r4

	sts.l	pr, @-r15
	mov.l	2f, r0
	jsr	@r0
	nop
	lds.l	@r15+, pr
	rte
	nop

	.zero 6

2:	.long	_debug
1:	.long	0xff000028
#endif

/* SH7305-TYPE INTERRUPT HANDLER ENTRY - 40 BYTES */

_gint_inth_7305:
	/* Save caller-saved registers which might currently be in use by the
	   interrupted function, as we don't know what the callback will do */
	sts.l	pr, @-r15
	stc.l	gbr, @-r15
	sts.l	mach, @-r15
	sts.l	macl, @-r15

	/* Get the event code from the INTEVT register */
	mov.l	1f, r0
	mov.l	@r0, r0

	/* Interrupt codes start at 0x400 */
	mov	#4, r1
	shll8	r1
	sub	r1, r0

	/* Add the distance to the first entry and jump as a subroutine */
	add	#40, r0
	bsrf	r0
	nop

	/* Restore caller-saved registers */
	lds.l	@r15+, macl
	lds.l	@r15+, mach
	ldc.l	@r15+, gbr
	lds.l	@r15+, pr

	rte
	nop

	.zero	24
1:	.long	0xff000028

#ifdef FX9860G

/* SH7705-TYPE INTERRUT HANDLER ENTRY - 52 BYTES */

_gint_inth_7705:
	/* Save caller-saved registers as before */
	sts.l	pr, @-r15
	stc.l	gbr, @-r15
	sts.l	mach, @-r15
	sts.l	macl, @-r15

	/* Get the event code from the INTEVT2 register */
	mov.l	1f, r0
	mov.l	@r0, r0		/* r0 = old_code */

	/* Translate the event code to SH4 format */
	mov.l	2f, r2
	mov	#-5, r3
	shld	r3, r0		/* r0 = old_code >> 5 */
	add	#-32, r0	/* r0 = (old_code - 0x400) >> 5 */
	mov.b	@(r0, r2), r0	/* r0 = (new_code - 0x400) >> 5 */
	mov	#5, r3
	shld	r3, r0		/* r0 = new_code - 0x400 */

	/* Add the distance between nop and the first entry, and jump */
	add	#32, r0
	bsrf	r0
	nop

	/* Restore saved registers */
	lds.l	@r15+, mach
	lds.l	@r15+, macl
	ldc.l	@r15+, gbr
	lds.l	@r15+, pr

	rte
	nop

	.zero 12
1:	.long	0xa4000000	/* INTEVT2 register */
2:	.long	_inth_remap

.section .gint.data
.align 4

/* EVENT CODE TRANSLATION TABLE - 96 BYTES */

_inth_remap:
	.byte	0x00, 0x01, 0x02, 0xfe, 0x34, 0x35, 0x36, 0xff
	.byte	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f
	.byte	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0xff, 0xff
	.byte	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	.byte	0x20, 0x21, 0x22, 0x23, 0x28, 0x28, 0xff, 0x28
	.byte	0x48, 0x48, 0xff, 0x48, 0xff, 0xff, 0xff, 0xff
	.byte	0xff, 0x31, 0x32, 0x33, 0xff, 0xff, 0xff, 0xff
	.byte	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	.byte	0x2d, 0x2d, 0xff, 0xff, 0x2d, 0x2d, 0xff, 0xff
	.byte	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	.byte	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	.byte	0x2f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff

#endif
