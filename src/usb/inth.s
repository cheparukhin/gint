/*
**  gint:usb:inth - Interrupt handler for the USB function module
*/

.global _inth_usb

.section .gint.blocks, "ax"
.align 4

/* USB INTERRUPT HANDLER */

_inth_usb:
	/* Call back into driver code, there is way too much work that needs to
	   be done to fit in here */
	sts.l	pr, @-r15
	mov.l	1f, r0
	mov.l	@r0, r0
	mov.l	2f, r4
	jsr	@r0
	nop

	lds.l	@r15+, pr
	rts
	nop

.zero 6

1:	.long _gint_inth_callback
2:	.long _usb_interrupt_handler
