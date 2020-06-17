/*
**	gint:core:vbr - Assembler-level VBR management
*/

.global	_gint_setvbr
.section .gint.mapped

/* gint_setvbr()
   Changes the VBR address and the calls the configuration function while
   interrupts are disabled. The configuration function must change either the
   priority registers or the interrupt masks, and make sure that all the
   interrupts that it leaves enabled are handled by the new VBR handlers.

   @r4  vbr        New VBR address (uint32_t)
   @r5  configure  Configuration function (void -> void)
   Returns the previous VBR address. */
_gint_setvbr:
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15

	mov	#1, r8
	mov	#28, r0
	shld	r0, r8

	/* Block all interrupts */
	stc	sr, r0
	or	r8, r0
	ldc	r0, sr

	/* Set the new VBR address */
	stc	vbr, r9
	ldc	r4, vbr

	/* Call the configuration function */
	jsr	@r5
	nop

	/* Enable interrupts again */
	stc	sr, r0
	not	r8, r8
	and	r8, r0
	ldc	r0, sr

	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov	r9, r0
	rts
	mov.l	@r15+, r9
