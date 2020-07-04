.global _memset
.text

_memset:
	tst	r6, r6
	bt	.zero

	/* We'll fill from the end */
	mov	r4, r3
	add	r6, r4

	/* When setting less than 64 bytes, use the naive method */
	mov	#64, r0
	cmp/ge	r6, r0
	bt	_naive_memset

	mov	#3, r2

	/* Make a 4-byte filler */
	mov	r5, r0
	shll8	r5
	or	r5, r0
	mov	r0, r5
	shll16	r5
	or	r5, r0

_memset_align:
	/* 4-align the destination */
	mov.b	r0, @-r4
	tst	r2, r4
	bf/s	_memset_align
	dt	r6

	mov	#8, r2

.aligned4:
	mov.l	r0, @-r4
	cmp/ge	r6, r2
	bf/s	.aligned4
	add	#-4, r6

_naive_memset:
	/* Tight loop copy one byte */
	dt	r6
	bf/s	_naive_memset
	mov.b	r5, @-r4

.end:
	rts
	mov	r3, r0

.zero:
	rts
	mov	r4, r0
