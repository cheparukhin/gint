/*
**  gint:tmu:inth-etmu - Interrupt handlers for the RTC-bound timers
*/

/* Gates for the extra timers (informally called ETMU) */
.global _inth_etmu4
.global _inth_etmux

.section .gint.blocks, "ax"
.align 4

/* EXTRA TMU INTERRUPT HANDLERS - 96 BYTES
   To implement the same functionalities as the standard timers, several blocks
   are once again needed. This time, 2 empty blocks after ETMU4 (0xd20, 0xd40)
   are used for convenience.

   It would be possible to communicate between any interrupt handlers in non-
   consecutive gates. A simple way is to store at runtime a pointer to the
   desired object in one handler. But that costs a lot fo space. If the
   relative position of interrupt handlers is known, the best option left is
   the unnatural @(disp,pc) addressing mode, and it doesn't even work with the
   SH3's compact VBR scheme. */

/* FIRST GATE - ETMU4 and two empty blocks */
_inth_etmu4:
	mova	.storage_etmu4, r0
	mov	#7, r2

.shared:
	mov.l	r2, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r0, r1

	/* Clear interrupt flag in TCR */
	mov.l	@(8, r1), r3
1:
	mov.b	@r3, r0
	tst	#0x02, r0
	and	#0xfd, r0
	bf/s	1b
	mov.b	r0, @r3

	/* Prepare invoking the callback function */
	mov.l	.gint_inth_callback, r8
	mov.l	@r8, r8
	mov.l	@r1, r4
	jsr	@r8
	mov.l	@(4, r1), r5
	tst	r0, r0
	bt	2f

	/* Invoke callback; if return value is non-zero, stop timer */
	mov.l	.timer_stop, r4
	jsr	@r8
	mov.l	@(8, r15), r5

	/* Clear the flag and possibly stop the timer */
2:
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	rts
	add	#4, r15

	.zero 24

.gint_inth_callback:
	.long	_gint_inth_callback
.timer_stop:
	.long	_timer_stop

.storage_etmu4:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0		/* TCR: Configured dynamically */

/* SECOND GATE - All other ETMU entries, falling back to ETMU2 */
_inth_etmux:
	/* Dynamically compute the target of the jump */
	stc	vbr, r3
	mov.l	1f, r2
	add	r2, r3

	mova	.storage_etmux, r0
	mov.w	.id_etmux, r2
	jmp	@r3
	nop

.id_etmux:
	.word	0		/* Timer ID */

	/* Offset from VBR where extra timer 2 is located:
	   * 0x600 to reach the interrupt handlers
	   * 0x040 to jump over the entry gate
	   * 0x900 to reach the handler of ETMU4
	   * Skip over the first instructions
	   This is different on SH3 due to the compact scheme so it's edited
	   dynamically at install time. */
1:	.long	0xf40 + (.shared - _inth_etmu4)

.storage_etmux:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0		/* TCR: Configured dynamically */
