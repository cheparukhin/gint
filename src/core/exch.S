/*
**	gint:core:exch - Exception handlers
*/

.global _exch_entry_7305

#ifdef FX9860G
.global _exch_entry_7705
#endif

.section .gint.blocks, "ax"
.align 4

/* SH7305-TYPE DEBUG EXCEPTION HANDLER - 26 BYTES */

_exch_entry_7705:
_exch_entry_7305:
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

2:	.long	_debug_exc
1:	.long	0xff000024
