.global _memcpy
.text

_memcpy:
	tst	r6, r6
	bt	.zero

	mov	r4, r3
	mov	#3, r2

	/* When copying less than 64 bytes, use the naive method */
	mov	#64, r0
	cmp/ge	r6, r0
	bt	_naive_memcpy

_memcpy_align_dst:
	/* 4-align the destination */
	mov.b	@r5+, r0
	mov.b	r0, @r4
	add	#1, r4
	tst	r2, r4
	bf/s	_memcpy_align_dst
	dt	r6

	/* If source is 4-aligned, use mov.l */
	tst	r2, r5
	bt/s	.aligned4
	mov	#4, r2

	/* If unaligned but SH4, use movua.l */
	mov.l	.gint, r0
	mov.l	@r0, r0
	tst	#1, r0
	bt	.unaligned4

	/* If source is 2-aligned, use mov.w */
	mov	r5, r0
	tst	#1, r0
	bt	.aligned2

	/* Otherwise use a naive copy */
	bra	_naive_memcpy
	nop

.aligned4:
	/* Copy 4 bytes at a time until at most 4 bytes are left */
	mov.l	@r5+, r0
	mov.l	r0, @r4
	add	#-4, r6
	cmp/ge	r6, r2
	bf/s	.aligned4
	add	#4, r4

	bra	_naive_memcpy
	nop

.unaligned4:
	/* Copy 4 bytes but read with movua.l since source is unaligned */
	movua.l	@r5+, r0
	mov.l	r0, @r4
	add	#-4, r6
	cmp/ge	r6, r2
	bf/s	.unaligned4
	add	#4, r4

	bra	_naive_memcpy
	nop

.aligned2:
	mov.w	@r5+, r0
	mov.w	r0, @r4
	mov.w	@r5+, r0
	mov.w	r0, @(2,r4)
	add	#-4, r6
	cmp/ge	r6, r2
	bf/s	.aligned2
	add	#4, r4

	bra	_naive_memcpy
	nop

_naive_memcpy:
	mov.b	@r5+, r0
	dt	r6
	mov.b	r0, @r4
	bf/s	_naive_memcpy
	add	#1, r4

	rts
	mov	r3, r0

.zero:
	rts
	mov	r4, r0

.align 4
.gint:
	.long _gint
