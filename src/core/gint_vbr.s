/*
	gint_vbr

	Some of the work, especially related to setting and un-setting the vbr
	address needs to be done in assembler.
*/

	.global	_gint_getVBR
	.global	_gint_setVBR



/*
	gint_getVBR()
	Returns the current vbr address.
*/
_gint_getVBR:
	rts
	stc	vbr, r0



/*
	gint_setVBR()

	This is quite the hard part when modifying the vbr. We need to set
	immediately the interrupt priorities of our own handler, or restore
	the ones used by the system ; otherwise we may receive interrupts
	requests that the new handler doesn't handle, which will cause the
	whole program to freeze.

	Therefore, we must set vbr *and* change interrupt priorities while
	having disabled all the interrupts in the status register. That's why
	this function takes as parameter the priority management function.
*/
_gint_setVBR:
	sts.l	pr, @-r15

	/* Blocking all interrupts. */
	mov.l	sr_block, r0
	stc	sr, r3
	or	r0, r3
	ldc	r3, sr

	/* Setting the vbr address. */
	ldc	r4, vbr

	/* Calling the priority manager. */
	jsr	@r5
	nop

	/* Activating interrupts again. */
	mov.l	sr_block, r0
	not	r0, r0
	stc	sr, r3
	and	r0, r3
	ldc	r3, sr

	lds.l	@r15+, pr
	rts
	nop

	.align	4

sr_block:
	.long	(1 << 28)
