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

/* OS version */
.global ___os_version

/* Dynamic allocation */
.global _malloc
.global _free
.global _calloc
.global _realloc

/* Bfile driver */
.global	_BFile_Remove
.global	_BFile_Create
.global	_BFile_Open
.global	_BFile_Close
.global	_BFile_Write
.global	_BFile_Read

.section ".pretext"

#define syscall(id)			\
	mov.l	syscall_table, r2	;\
	mov.l	1f, r0			;\
	jmp	@r2			;\
	nop				;\
1:	.long	id

#ifdef FX9860G

/* OS version */

___os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x02ee

/* Dynamic allocation */

_malloc:
	syscall(0x0acd)
_free:
	syscall(0x0acc)
_calloc:
	syscall(0x0e6b)
_realloc:
	syscall(0x0e6d)

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

/* OS version */

___os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x1406

/* Dynamic allocation */

_malloc:
	syscall(0x1f44)
_free:
	syscall(0x1f42)
_calloc:
	syscall(0x1f40)
_realloc:
	syscall(0x1f46)

syscall_table:
	.long	0x80020070

#endif /* FXCG50 */
