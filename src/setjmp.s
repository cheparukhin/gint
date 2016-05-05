/*
	This file implements long jumps. An example of their use is with crt0.c
	and exit()-family functions that use them to restore the execution
	state when leaving the program from an unknown location.

	The register contents are saved in a buffer when setjmp() is called and
	restored at any time when longjmp() performs the jump, through an
	exit() or abort() call, for instance.

	This is actually a question of playing with pr ; the user program is
	resumed after the setjmp() call when longjmp() is invoked but setjmp()
	has nothing to do with this operation. longjmp() restores the pr value
	that was saved by setjmp() and performs an rts instruction.

	setjmp() returns 0 when called to set up the jump point and a non-zero
	value when invoked through a long jump. If 0 is given as return value
	to longjmp(), then 1 is returned.
*/

	.global	_setjmp
	.global	_longjmp

_setjmp:
	/* Getting some free space. */
	add	#64, r4

	/* Saving general-purpose registers. */
	mov.l	r15, @-r4
	mov.l	r14, @-r4
	mov.l	r13, @-r4
	mov.l	r12, @-r4
	mov.l	r11, @-r4
	mov.l	r10, @-r4
	mov.l	r9, @-r4
	mov.l	r8, @-r4

	/* Saving control and system registers. */
	stc.l	sr, @-r4
	stc.l	ssr, @-r4
	stc.l	spc, @-r4
	stc.l	gbr, @-r4
	stc.l	vbr, @-r4
	sts.l	mach, @-r4
	sts.l	macl, @-r4
	sts.l	pr, @-r4

	/* This function always return 0. The cases where setjmp() seems to
	return non-zero values, when a long jump has just been performed, is
	actually handled by longjmp(). */
	rts
	mov	#0, r0



_longjmp:
	/* Restoring the system and control registers. Restoring pr is actually
	what performs the jump -- and makes the user program thinks that
	setjmp() has returned. */
	lds.l	@r4+, pr
	lds.l	@r4+, macl
	lds.l	@r4+, mach
	ldc.l	@r4+, vbr
	ldc.l	@r4+, gbr
	ldc.l	@r4+, spc
	ldc.l	@r4+, ssr
	ldc.l	@r4+, sr

	/* Restoring the general-purpose registers. */
	mov.l	@r4+, r8
	mov.l	@r4+, r9
	mov.l	@r4+, r10
	mov.l	@r4+, r11
	mov.l	@r4+, r12
	mov.l	@r4+, r13
	mov.l	@r4+, r14
	mov.l	@r4+, r15

	/* Preventing return value from being 0 (must be at least 1). */
	tst	r5, r5
	movt	r0
	rts
	add	r5, r0
