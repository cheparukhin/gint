/*
**	gint:core:syscalls - calls to CASIOWIN
**
**	This files can be seen as a list of everywhere gint relies on the
**	underlying OS. Although I wish to make gint free-standing, there are
**	still a few hard dependencies, namely:
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

.section ".pretext"

#ifdef FX9860G

/* OS version */

___os_version:
	mov.l	syscall_table, r2
	mov.l	1f, r0
	jmp	@r2
	nop
1:	.long	0x02ee

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
