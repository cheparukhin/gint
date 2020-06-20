/*
**  gint:tmu:inth - Interrupt handlers for the timer units
**  Perhaps the most technical of my interrupt handlers. They implement a
**  simple kind of interrupt handler communication by letting the control flow
**  from each interrupt handler to the next.
*/

/* Gates for the standard Timer Unit (TMU) */
.global _inth_tmu         /* 128 bytes */

/* Gates for the extra timers (informally called ETMU) */
.global _inth_etmu2       /* 32 bytes */
.global _inth_etmu_help   /* 32 bytes */
.global _inth_etmux       /* 32 bytes */

.section .gint.blocks, "ax"
.align 4

/* TMU INTERRUPT HANDLERS - 128 BYTES
   Unfortunately I did not manage to write a handler that cleared the interrupt
   flag and invoked a callback in less than 34 bytes data included. So I
   decided to make several gates operate as a whole and add a bit more features
   in them. Basically, these handlers:
   - Clear the interrupt flag
   - Invoke a callback function and pass it a user-provided argument
   - Stop the timer if the callback returns non-zero
   - Host their own callback pointers and arguments

   It is important to notice that the code of the following gates looks like
   they are contiguous in memory. The assembler will make that assumption, and
   turn any address reference between two gates into a *relative displacement*.
   If the gates don't have the same relative location at runtime, the code will
   crash because we will have broken the references. This is why we can only do
   it with handlers that are mapped to consecutive event codes. */

_inth_tmu:

/* FIRST GATE - TMU0 entry, clear underflow flag and call back */
_inth_tmu_0:
	mova	.storage0, r0
	mov	#0, r1

/*** This is the first shared section ***/
.shared1:
	sts.l	pr, @-r15
	mov.l	r1, @-r15

	/* Load the TCR address */
	mov.l	.mask, r3
	not	r3, r4
	mov.l	@(8, r0), r1

	/* Clear the interrupt flag */
1:	mov.w	@r1, r2
	tst	r4, r2
	and	r3, r2
	mov.w	r2, @r1
	bf	1b

	/* Prepare callback and jump to second section */
	mov.l	.gint_inth_callback_1, r1
	mov.l	@r0, r4
	bra	.shared2
	mov.l	@(4, r0), r5

/* SECOND GATE - TMU1 entry and stop timer */
_inth_tmu_1:
	mova	.storage1, r0
	bra	.shared1
	mov	#1, r1

/*** This is the second shared section ***/
.shared2:
	/* Invoke callback */
	jsr	@r1
	nop

	/* Stop the timer if the return value is not zero */
	mov.l	@r15+, r4
	tst	r0, r0
	bt	.shared3
	mov.l	.timer_stop_1, r1
	jsr	@r1
	nop

.shared3:
	lds.l	@r15+, pr
	rts
	nop

	.zero	4

/* THIRD GATE - TMU2 entry and storage for TMU0 */
_inth_tmu_2:
	mova	.storage2, r0
	bra	.shared1
	mov	#2, r1

	.zero	10

.gint_inth_callback_1:
	.long	_gint_inth_callback

.storage0:
	.long	0		 /* Callback: Configured dynamically */
	.long	0		 /* Argument: Configured dynamically */
	.long	0xa4490010	 /* TCR0: Overridden at startup on SH3 */

/* FOURTH GATE - Storage for TMU1, TMU2 and other values */
_inth_tmu_storage:

.mask:
	.long	0xfffffeff
.timer_stop_1:
	.long	_timer_stop

.storage1:
	.long	0 		/* Callback: Configured dynamically */
	.long	0 		/* Argument: Configured dynamically */
	.long	0xa449001c	/* TCR1: Overridden at startup on SH3 */

.storage2:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0xa4490028	/* TCR2: Overridden at startup on SH3 */



/* EXTRA TMU INTERRUPT HANDLERS - 96 BYTES
   To implement the same functionalities as the standard timers, several blocks
   are once again needed. But the handlers for the extra timers are not located
   in adjacent gates, except for ETMU1 and ETMU2 which have event codes 0xc20
   and 0xc40. Since handler 0xc60 is free on SH4 and not redirected to on SH3,
   I use it to build a three-handler block similar to that of the TMU above.

   On SH4 this means that an extra gate has to be installed, but no interrupt
   point here. On SH3 this means that four gates are used for the only extra
   timer, but the incurred cost is minimal (96 bytes on the binary file)
   because the size of the VBR area can hardly be shrunk anyway.

   It *is* possible to do generalized communication between interrupt handlers
   that do not reside in consecutive gates. The general way of performing a
   jump or data access between two interrupt handlers would be to store at
   runtime the address of the target resource in a reserved longword in the
   source handler. But longwords are costly in 32-byte areas. Even if the event
   codes of the interrupt handlers are known at development time, the best I
   can think of is hardcoding the relative displacements, and one would need to
   use the unnatural and unmaintainable @(disp, pc) addressing modes. */

/* FIRST GATE - ETMU2 entry, invoke callback and prepare clear flag */
_inth_etmu2:
	mova	.storage_etmu2, r0
	mov	#5, r1

.shared4:
	mov.l	r1, @-r15
	sts.l	pr, @-r15
	mov.l	r0, @-r15

	/* Clear interrupt flag */
	mov.l	.timer_clear, r2
	jsr	@r2
	mov	r1, r4

	/* Prepare invoking the callback function */
	mov.l	@r15+, r0
	mov.l	.gint_inth_callback_2, r1
	bra	_inth_etmu_help
	mov.l	@r0, r4

.storage_etmu2:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */

/* SECOND GATE - Helper entry, invoke callback and stop timer if requested */
_inth_etmu_help:

	/* Invoke callback; if return value is non-zero, stop timer */
	jsr	@r1
	mov.l	@(4, r0), r5
	tst	r0, r0
	bt	.shared5
	mov.l	.timer_stop_2, r1
	jsr	@r1
	mov.l	@(4, r15), r4

	/* Clear the flag and possibly stop the timer */

.shared5:
	lds.l	@r15+, pr
	rts
	add	#4, r15

.gint_inth_callback_2:
	.long	_gint_inth_callback
.timer_clear:
	.long	_timer_clear
.timer_stop_2:
	.long	_timer_stop

/* THIRD GATE - All other ETMU entries, deferred to the previous ones */
_inth_etmux:
	/* Dynamically compute the target of the jump */
	stc	vbr, r3
	mov.l	1f, r2
	add	r2, r3

	mova	.storage_etmux, r0
	mov.l	.id_etmux, r1
	jmp	@r3
	nop
	nop

	/* Offset from VBR where extra timer 2 is located:
	   * 0x600 to reach the interrupt handlers
	   * 0x040 to jump over the entry gate
	   * 0x840 to reach the handler of ETMU2
	   * Skip over the first instructions */
1:	.long	0xe80 + (.shared4 - _inth_etmu2)

.id_etmux:
	.long	0		/* Timer ID */
.storage_etmux:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
