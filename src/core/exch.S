/*
**	gint:core:exch - Exception handlers
*/

.global _exch_entry_7305

#ifdef FX9860G
.global _exch_entry_7705
#endif

.section .gint.blocks, "ax"
.align 4

/* SH7305-TYPE DEBUG EXCEPTION HANDLER */

_exch_entry_7705:
_exch_entry_7305:
	mov.l	1f, r0
	mov.l	@r0, r4

	rte
	nop

	.zero 20

1:	.long	0xff000024
