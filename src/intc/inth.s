/* gint.intc.inth: Standard interrupt handler

   This is a generic interrupt handler that calls back into a C function,
   useful for complex handling or simple drivers that benefit more from
   simplicity than razor-sharp performance. */

.global _intc_generic_handler /* 32 bytes */

.section .gint.blocks, "ax"
.align 4

_intc_generic_handler:
	/* Create a GINT_CALL() object with no arguments */
	sts.l	pr, @-r15
	add	#-20, r15
	mov.l	.function, r0
	mov.l	r0, @r15

	/* Call back to the function in userspace */
	mov.l	.gint_inth_callback, r0
	mov.l	@r0, r0
	jsr	@r0
	mov	r15, r4

	add	#20, r15
	lds.l	@r15+, pr
	rts
	nop

.function:
	.long	0 /* Function, set when handler is installed */
.gint_inth_callback:
	.long	_gint_inth_callback
