/*
	gint core module: syscalls

	All the system calls used by the library. Somehow "the less, the
	better".
	We have finally gotten rid of every obscure system-related syscalls!
	Looks like an important step towards being completely free-standing :)
*/

	.global	___malloc
	.global	___free
	.global	___realloc
	.global	___system_menu



___malloc:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacd

___free:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacc

___realloc:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xe6d

/*
	__system_menu()
	Brings one back to the system menu by putting KEY_MENU in the system's
	key buffer and calling GetKeyWait(). Of course this needs to be
	executed while under system control.
*/
___system_menu:
	sts.l	pr, @-r15
	add	#-4, r15

	/* Putting the matrix code in the key buffer. */

	mov	r15, r4
	mov.w	.matrix_menu, r2
	mov.w	r2, @r4
	mov.l	syscall_table, r1
	mov.l	.syscall_putcode, r0
	jsr	@r1
	nop

	/* Calling GetKeyWait() to display menu. */

	mov	r15, r4				/* column pointer */
	add	#-4, r15
	mov	r15, r5				/* row pointer */
	mov	#-5, r15
	mov	r15, r1

	mov	#2, r6				/* type of waiting */
	mov	#0, r7				/* timeout period */
	mov.l	r1, @-r15			/* keycode pointer */
	mov	#0, r2
	mov.l	r2, @-r15			/* allow return to menu */

	mov.l	syscall_table, r1
	mov.l	.syscall_getkeywait, r0
	jsr	@r1
	nop

	/* If the program counter reaches this place, it means that the user
	   has come back to the program. Restore stack and leave. */

	add	#20, r15
	lds.l	@r15+, pr
	rts
	nop

	.align	4

.syscall_getkeywait:
	.long	0x0247
.syscall_putcode:
	.long	0x024f
.matrix_menu:
	.word	0x0308



	.align 4

syscall_table:
	.long	0x80010070
