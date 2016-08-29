/*
	gint core module: syscalls

	All the system calls used by the library. Somehow "the less, the
	better".
*/

	.global	___GLibAddinAplExecutionCheck
	.global	_malloc
	.global	_free



___GLibAddinAplExecutionCheck:
	mov.l	syscall_table, r2
	mov	#0x13, r0
	jmp	@r2
	nop

_malloc:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacd

_free:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacc



	.align	4

syscall_table:
	.long	0x80010070
