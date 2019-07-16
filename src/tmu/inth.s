/*
**  gint:tmu:inth - Interrupt handlers for the timer units
**  Perhaps the most technical of my interrupt handlers. They implement a
**  simple kind of interrupt handler communication by letting the control flow
**  from each interrupt handler to the next.
*/

/* Gates for the standard Timer Unit (TMU) */
.global _inth_tmu_0
.global _inth_tmu_1
.global _inth_tmu_2
.global _inth_tmu_storage

/* Gates for the extra timers (informally called ETMU) */
.global _inth_etmu2
.global _inth_etmu_help
.global _inth_etmux

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

/* FIRST GATE - TMU0 entry, clear underflow flag and call back */
_inth_tmu_0:
	mova	.storage0, r0
	mov	#0, r1

	/*** This is the first shared section ***/

.clearflag:
	sts.l	pr, @-r15
	mov.l	r1, @-r15

	/* Load the TCR address and clear the interrupt flag */
	mov.l	.mask, r3
	mov.l	@(8, r0), r1
	mov.w	@r1, r2
	and	r3, r2
	mov.w	r2, @r1

	/* Invoke the callback function and pass the argument */
	mov.l	@r0, r1
	jsr	@r1
	mov.l	@(4, r0), r4

	/* Prepare stopping the timer and jump to second section */
	mov.l	@r15+, r4
	mov.l	.timer_stop, r1
	bra	.stoptimer
	nop

/* SECOND GATE - TMU1 entry and stop timer */
_inth_tmu_1:
	mova	.storage1, r0
	bra	.clearflag
	mov	#1, r1

	/*** This is the second shared section ***/

.stoptimer:
	/* Stop the timer if the return value is not zero */
	tst	r0, r0
	bt	.end
	jsr	@r1
	nop

.end:
	lds.l	@r15+, pr
	rte
	nop

	.zero	12

/* THIRD GATE - TMU2 entry and storage for TMU0 */
_inth_tmu_2:
	mova	.storage2, r0
	bra	.clearflag
	mov	#2, r1

	.zero	14

.storage0:
	.long	0		 /* Callback: Configured dynamically */
	.long	0		 /* Argument: Configured dynamically */
	.long	0xa4490010	 /* TCR0: Overridden at startup on SH3 */

/* FOURTH GATE - Storage for TMU1, TMU2 and other values */
_inth_tmu_storage:

.mask:
	.long	0x0000feff
.timer_stop:
	.long	_timer_stop	/* gint's function from <gint/timer.h> */

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
	/* Warning: the size of the following section (4 bytes) is hardcoded in
	   the jump in _inth_etmux */
	mova	.storage_etmu2, r0
	mov	#5, r1

.extra_callback:
	sts.l	pr, @-r15
	mov.l	r1, @-r15

	/* Invoke the callback function */
	mov.l	@r0, r1
	jsr	@r1
	mov.l	@(4, r0), r4

	/* Load timer ID and forward the callback's return value */
	mov.l	.timer_clear, r1
	mov.l	@r15+, r4
	bra	_inth_etmu_help
	mov	r0, r5

	nop

.storage_etmu2:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */

/* SECOND GATE - Helper entry, invoke callback and stop timer if requested */
_inth_etmu_help:

	/* Clear the flag and possibly stop the timer */
	jsr	@r1
	nop

	lds.l	@r15+, pr
	rte
	nop

	.zero	18

.timer_clear:
	.long	_timer_clear	/* gint's function from src/tmu/tmu.c */

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
	   - 0x600 to reach the interrupt handlers
	   - 0x020 to jump over the entry gate
	   - 0x840 to reach the handler of ETMU2
	   - 0x004 to skip its first instructions (the size is hardcoded) */
1:	.long	0xe64

.storage_etmux:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
.id_etmux:
	.long	0		/* Timer ID */
