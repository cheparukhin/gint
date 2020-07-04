.global _memmove
.text

_memmove:
	tst	r6, r6
	bt	.zero

	/* Simple optimization: if regions do not overlap, use memcpy() */
	mov	r4, r0
	add	r6, r0
	cmp/ge	r0, r5
	bt	_memmove_memcpy
	mov	r5, r0
	add	r6, r0
	cmp/ge	r0, r4
	bt	_memmove_memcpy

	mov	r4, r3

	cmp/ge	r4, r5
	bf	.backwards

.forwards:
	/* If the destination starts before the source, copy forwards */
	mov.b	@r5+, r0
	mov.b	r0, @r4
	dt	r6
	bf/s	.forwards
	add	#1, r4

	rts
	mov	r3, r0

.backwards:
	/* Otherwise, copy backwards */
	add	r6, r4
	add	r6, r5

.backwards_loop:
	add	#-1, r5
	mov.b	@r5, r0
	dt	r6
	bf/s	.backwards_loop
	mov.b	r0, @-r4

	rts
	mov	r3, r0

_memmove_memcpy:
	mov.l	.memcpy, r1
	jmp	@r1
	nop

.zero:
	rts
	mov	r4, r0

.align 4
.memcpy:
	.long _memcpy
