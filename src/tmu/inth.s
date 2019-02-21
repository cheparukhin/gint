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
	.global _inth_tmu_extra2
	.global _inth_tmu_extra_help
	.global _inth_tmu_extra_others

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
	mov	#-2, r1

	/*** This is the first shared section ***/

.clearflag:
	mov.l	r1, @-r15

	/* Load the TCR address and clear the interrupt flag */
	mov.l	.mask, r3
	mov.l	@(8, r0), r1
	mov.w	@r1, r2
	and	r3, r2
	mov.w	r2, @r1

	/* Invoke the callback function and pass the argument */
	sts.l	pr, @-r15
	mov.l	@r0, r1
	jsr	@r1
	mov.l	@(4, r0), r4
	lds.l	@r15+, pr

	/* Prepare stopping the timer and jump to second section */
	mov.l	.tstr, r5
	bra	.stoptimer
	mov.l	@r15+, r1

/* SECOND GATE - TMU1 entry and stop timer */
_inth_tmu_1:
	mova	.storage1, r0
	bra	.clearflag
	mov	#-3, r1

	/*** This is the second shared section ***/

.stoptimer:
	/* Stop the timer if the return value is not zero */
	tst	r0, r0
	bt	.end
	mov.b	@r5, r2
	and	r1, r2
	mov.b	r2, @r5

.end:
	rte
	nop

	.zero	12

/* THIRD GATE - TMU2 entry and storage for TMU0 */
_inth_tmu_2:
	mova	.storage2, r0
	bra	.clearflag
	mov	#-5, r1

	.zero	14

.storage0:
	.long	0		 /* Callback: Configured dynamically */
	.long	0		 /* Argument: Configured dynamically */
	.long	0xa4490010	 /* TCR0: Overridden at startup on SH3 */

/* FOURTH GATE - Storage for TMU1, TMU2 and other values */
_inth_tmu_storage:

.mask:	.long	0x0000feff
.tstr:	.long	0xa4490004	/* TSTR: Overridden at startup on SH3 */

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

/* FIRST GATE - ETMU2 entry, clear flag and prepare callback */
_inth_tmu_extra2:
	/* Warning: the size of the following instruction (2 bytes) is
	   hardcoded in another interrupt handler, _inth_tmu_extra_others */
	mova	.storage_extra_1, r0

.extra_callback:
	mov.l	r8, @-r15
	mov	r0, r8
	stc.l	gbr, @-r15

	/* Invoke the callback function */
	sts.l	pr, @-r15
	mov.l	@r8, r1
	jsr	@r1
	mov.l	@(4, r8), r4

	bra	.extra_clearflag
	lds.l	@r15+, pr

.storage_extra_1:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0		/* Structure address: Edited at startup */

/* SECOND GATE - Helper entry, invoke callback and stop timer if requested */
_inth_tmu_extra_help:

.extra_clearflag:
	mov	r0, r5

	/* Load struture address */
	mov.l	@(8, r8), r0
	ldc	r0, gbr
	mov	#12, r0

.extra_loopclear:
	/* Aggressively clear the interrupt flag. The loop is required because
	   clearing takes "some time". The system does it. Not doing it will
	   cause the interrupt to be triggered again until the flag is cleared,
	   which can be ~20 interrupts if the handler is fast! */
	and.b	#0xfd, @(r0, gbr)
	tst.b	#0x02, @(r0, gbr)
	bf	.extra_loopclear

	/* Check whether to stop the timer */
	tst	r5, r5
	bt	.extra_end

.extra_stoptimer:
	mov.b	@(0, gbr), r0
	and	#0xfe, r0
	mov.b	r0, @(0, gbr)

.extra_end:
	ldc.l	@r15+, gbr
	mov.l	@r15+, r8
	rte
	nop

/* FOURTH GATE - All other ETMU entries, deferred to the previous ones */
_inth_tmu_extra_others:
	/* Dynamically compute the target of the jump */
	stc	vbr, r1
	mov.l	1f, r2
	add	r2, r1

	mova	.storage_extra_others, r0
	jmp	@r1
	nop

	/* Offset from VBR where extra timer 2 is located:
	   - 0x600 to reach the interrupt handlers
	   - 0x020 to jump over the entry gate
	   - 0x840 to reach the handler of extra timer 2
	   - 0x002 to skip its first instruction (the size is hardcoded) */
1:	.long	0xe62

	.zero 4

.storage_extra_others:
	.long	0		/* Callback: Configured dynamically */
	.long	0		/* Argument: Configured dynamically */
	.long	0		/* Structure address: Edited at startup */
