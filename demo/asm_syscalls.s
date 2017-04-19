
	# int BFile_Remove(const uint16_t *file)
	.global	_BFile_Remove

_BFile_Remove:
	mov.l	1f, r0
	mov.l	2f, r1
	jmp	@r1
	mov	#0, r5
1:	.long	0x0439

	# int BFile_Create(const uint16_t *file, enum { file = 1, folder = 5 },
	# int *size)
	.global	_BFile_Create

_BFile_Create:
	mov.l	1f, r0
	mov.l	2f, r1
	jmp	@r1
	nop
1:	.long	0x0434

	# int BFile_Open(const uint16_t *file, int mode)
	.global	_BFile_Open

_BFile_Open:
	mov.l	1f, r0
	mov.l	2f, r1
	jmp	@r1
	mov	#0, r6
1:	.long	0x042c

	# int BFile_Close(int handle)
	.global	_BFile_Close

_BFile_Close:
	mov.l	1f, r0
	mov.l	2f, r1
	jmp	@r1
	nop
1:	.long	0x042d

	# int BFile_Write(int handle, void *ram_buffer, int even_size)
	.global	_BFile_Write

_BFile_Write:
	mov.l	1f, r0
	mov.l	2f, r1
	jmp	@r1
	nop
1:	.long	0x0435

	.align	4

# Syscall branch address
2:	.long	0x80010070
