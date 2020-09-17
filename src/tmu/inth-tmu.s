/*
**  gint:tmu:inth-tmu - Interrupt handlers for the timer units
**  Perhaps the most technical of my interrupt handlers. They implement a
**  simple kind of interrupt handler communication by letting the control flow
**  from each interrupt handler to the next.
*/

/* Gates for the standard Timer Unit (TMU) */
.global _inth_tmu         /* 128 bytes */

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
	mov.l	r8, @-r15
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
	mov.l	.gint_inth_callback, r8
	bra	.shared2
	mov.l	@r8, r8

/* SECOND GATE - TMU1 entry and stop timer */
_inth_tmu_1:
	mova	.storage1, r0
	bra	.shared1
	mov	#1, r1

/*** This is the second shared section ***/
.shared2:
	/* Invoke callback */
	mov.l	@r0, r4
	jsr	@r8
	mov.l	@(4, r0), r5

	/* Stop the timer if the return value is not zero */
	mov.l	@r15+, r5
	tst	r0, r0
	bt	2f
	mov.l	.timer_stop, r4
	jsr	@r8
	nop

2:
	lds.l	@r15+, pr
	rts
	mov.l	@r15+, r8

	.zero	2

/* THIRD GATE - TMU2 entry and storage for TMU0 */
_inth_tmu_2:
	mova	.storage2, r0
	bra	.shared1
	mov	#2, r1

	.zero	10

.gint_inth_callback:
	.long	_gint_inth_callback

.storage0:
	.long	0		 /* Callback: Configured dynamically */
	.long	0		 /* Argument: Configured dynamically */
	.long	0xa4490010	 /* TCR0: Overridden at startup on SH3 */

/* FOURTH GATE - Storage for TMU1, TMU2 and other values */
_inth_tmu_storage:

.mask:
	.long	0xfffffeff
.timer_stop:
	.long	_timer_stop

.storage1:
	.long	0 		/* Callback: Configured dynamically */
	.long	0 		/* Argument: Configured dynamically */
	.long	0xa449001c	/* TCR1: Overridden at startup on SH3 */

.storage2:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0xa4490028	/* TCR2: Overridden at startup on SH3 */
