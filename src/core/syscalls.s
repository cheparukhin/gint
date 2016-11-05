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


	.align	4

syscall_table:
	.long	0x80010070
