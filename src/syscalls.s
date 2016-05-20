/*
	This file contains all the system calls used by the library. Maybe one
	day we won't need them anymore ?
*/

	.global	___Hmem_SetMMU
	.global	___GLibAddinAplExecutionCheck
	.global	_malloc
	.global	_calloc
	.global	_free



___Hmem_SetMMU:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x3fa

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

_calloc:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xe6b

_free:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0xacc



	.align	4

syscall_table:
	.long	0x80010070
