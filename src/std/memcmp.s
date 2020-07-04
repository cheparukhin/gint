.global _memcmp
.text

_memcmp:
	tst	r6, r6
	bt	.zero

	/* When comparing less than 64 bytes, use the naive method */
	mov	#64, r0
	cmp/ge	r6, r0
	bt	_naive_memcmp

	mov	#4, r2
	mov	#3, r3

_memcmp_align_rhs:
	/* 4-align the right-hand side */
	mov.b	@r4+, r0
	mov.b	@r5+, r1
	cmp/eq	r0, r1
	bf/s	.end
	dt	r6
	tst	r3, r5
	bf	_memcmp_align_rhs

	/* If left-hand side is 4-aligned, use mov.l */
	tst	r3, r4
	bt	.aligned4

	/* If unaligned but SH4, use movua.l */
	mov.l	.gint, r0
	mov.l	@r0, r0
	tst	#1, r0
	bt	.unaligned4

	/* If left-hand side is 2-aligned, use mov.w and mov.l */
	mov	r4, r0
	tst	#1, r0
	bt	.aligned2

	/* Otherwise use a naive comparison */
	bra	_naive_memcmp
	nop

.aligned4:
	/* Compare 4 bytes at a time until at most 4 bytes are left */
	mov.l	@r4+, r0
	mov.l	@r5+, r1
	cmp/eq	r0, r1
	bf/s	_fail
	add	#-4, r6
	cmp/ge	r6, r2
	bf	.aligned4

	bra	_naive_memcmp
	nop

.unaligned4:
	/* Compare 4 bytes at a time until at most 4 bytes are left. Since
	   left-hand side is aligned, use movua.l */
	movua.l	@r4+, r0
	mov.l	@r5+, r1
	cmp/eq	r0, r1
	bf/s	_fail
	add	#-4, r6
	cmp/ge	r6, r2
	bf	.unaligned4

	bra	_naive_memcmp
	nop

.aligned2:
	/* Read 4 bytes from r4 in two steps */
	mov.w	@r4+, r0
	mov.l	@r5+, r1
	mov.w	@r4+, r2
	shll16	r0
	or	r2, r0
	cmp/eq	r0, r1
	bf/s	_fail
	add	#-4, r6
	cmp/ge	r6, r2
	bf	.aligned2

	bra	_naive_memcmp
	nop

_fail:
	/* Rewind 4 bytes to compare manually */
	add	#-4, r4
	add	#-4, r5
	add	#4, r6

_naive_memcmp:
	mov.b	@r4+, r0
	mov.b	@r5+, r1
	cmp/eq	r0, r1
	bf/s	.end
	dt	r6
	bf	_naive_memcmp

.end:
	extu.b	r0, r0
	extu.b	r1, r1
	rts
	sub	r1, r0

.zero:
	rts
	mov	#0, r0

.align 4
.gint:
	.long _gint
