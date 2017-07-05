/*
	gint core module: syscalls

	System calls (and the like) used by the library. The library should
	rely the least possible on the system, but sometimes using the syscalls
	is nothing of a nuisance.

	For instance, using the malloc()-family syscalls is a bit annoying
	because it "locks" many functionalities. On the other hand, getting the
	SaveDisp() buffer addresses to store data here is not a problem since
	such data can very easily be relocated to static RAM.
*/

	/* Dynamic allocation */
	.global	___malloc
	.global	___free
	.global	___realloc

	/* OS version */
	.global	___get_os_version

	/* Return to menu */
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

___get_os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x02ee

/*
	__system_menu()
	Brings one back to the system menu by putting KEY_MENU in the system's
	key buffer and calling GetKeyWait(). Of course this needs to be
	executed while under system control.
*/
___system_menu:
	sts.l	pr, @-r15
	add	#-4, r15

	/* Copying gint's VRAM into the system's. */
	mov.l	syscall_table, r1
	mov.l	.syscall_vram, r0
	jsr	@r1
	mov.l	r4, @r15			/* gint video ram */
	mov	r0, r4
	mov.l	@r15, r5
	mov.w	.vram_size, r6
	mov.l	.memcpy, r1
	jsr	@r1
	nop

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
	add	#-4, r15
	mov	r15, r1				/* keycode pointer */

	mov	#2, r6				/* type of waiting */
	mov	#0, r7				/* timeout period */
	mov.l	r1, @-r15
	mov	#0, r2				/* allow return to menu */
	mov.l	r2, @-r15

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
.syscall_vram:
	.long	0x0135
.matrix_menu:
	.word	0x0308
.vram_size:
	.word	1024
.memcpy:
	.long	_memcpy



	.align 4

syscall_table:
	.long	0x80010070
