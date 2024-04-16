#include <gint/config.h>
#if GINT_OS_CP

.section .text.header
.global _start_header

_start_header:
	mov.l	.start, r0
	jmp	@r0
	nop

.align 2
.start:
	.long	_start
.load_addr:
	.long	_start_header

#endif
