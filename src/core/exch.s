.global _gint_exch_tlbh
.section .gint.exch_tlbh
.align 4

_gint_exch_tlbh:
	sts.l	pr, @-r15
	mov.l	r8, @-r15

	mov.l	.expevt, r8

	/* Panic if the catcher is NULL */
	mov.l	.catcher, r0
	mov.l	@r0, r0
	tst	r0, r0
	bt	panic

	/* Leave if the catcher returns zero */
	jsr	@r0
	mov.l	@r8, r4
	tst	r0, r0
	bt	end

panic:
	/* RTE to the panic function, but manually, so that SPC is preserved */
	mov.l	@r8, r4
	ldc	r4, r4_bank

	mov.l	@r15+, r8
	lds.l	@r15+, pr

	/* Here we switch banks so r0..r7 change meaning! */
	stc	ssr, r0
	ldc	r0, sr

	mov.l	.panic, r0
	mov.l	@r0, r0
	jmp	@r0
	nop

end:
	mov.l	@r15+, r8
	lds.l	@r15+, pr
	rte
	nop

.align 4

.expevt:
	.long	0xff000024
.catcher:
	.long	_gint_exc_catcher
.panic:
	.long	_gint_exc_panic
