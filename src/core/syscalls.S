/*
**	gint:core:syscalls - calls to CASIOWIN
**
**	This file can be seen as a list of everywhere gint relies on the
**	underlying OS. Although I wish to make gint free-standing, there are
**	still a few hard questions, namely:
**	* MMU management, because doing it wrong might break the calculator.
**	* Dynamic allocation, because we can't trash the system heap.
**	* File system, because it's a mess and we might ruin the ROM.
*/

/* Dynamic allocation */
.global ___malloc
.global ___calloc
.global ___free
.global ___realloc
/* OS version, for debugging purposes */
.global ___os_version
/* Bfile calls */
.global	_BFile_Remove
.global	_BFile_Create
.global	_BFile_Open
.global	_BFile_Close
.global	_BFile_Write
.global	_BFile_Read

.section ".pretext"

#ifdef FX9860G

/* OS version */

___os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x02ee

/* BFile driver */

# int BFile_Remove(const uint16_t *file)
_BFile_Remove:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	mov	#0, r5
1:	.long	0x0439

# int BFile_Create(uint16_t *file, enum { file = 1, folder = 5 }, int *size)
_BFile_Create:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	nop
1:	.long	0x0434

# int BFile_Open(const uint16_t *file, int mode)
_BFile_Open:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	mov	#0, r6
1:	.long	0x042c

# int BFile_Close(int handle)
_BFile_Close:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	nop
1:	.long	0x042d

# int BFile_Write(int handle, const void *ram_buffer, int even_size)
_BFile_Write:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	nop
1:	.long	0x0435

# int BFile_Read(int handle, void *ram_buffer, int size, int whence)
_BFile_Read:
	mov.l	1f, r0
	mov.l	syscall_table, r1
	jmp	@r1
	nop
1:	.long	0x0432

syscall_table:
	.long	0x80010070

#endif /* FX9860G */

#ifdef FXCG50

___os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x1406

syscall_table:
	.long	0x80020070

#endif /* FXCG50 */
