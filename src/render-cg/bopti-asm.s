
.global _bopti_r5g6b5
.global _bopti_r5g6b5a

# REGISTER ALLOCATION:
# r0: (tmp)
# r1: (tmp)
# r2: width - 1
# r3: target & 2
# ---
# r4: data
# r5: target
# r6: width; then, the number of longword operations
# r7: height
# ---
# r8: in_stride
# r9: out_stride
# r10: x counter
# ---
# @12: in_stride
# @16: out_stride

.align 4

_bopti_r5g6b5:
	# Target alignment, either 0 (4-aligned) or 2 (2-aligned)
	mov.l	r8, @-r15
	mov	r5, r3
	mov.l	r9, @-r15
	mov	#2, r0
	mov.l	r10, @-r15
	and	r0, r3

	# width-1, used to copy the last longword
	mov	r6, r2
	add	#-1, r2
	shll	r2

	# Number of longword operations per row
	shlr	r6

	# Input and output strides, minus aligment
	mov.l	@(12, r15), r8
	mov.l	@(16, r15), r9
	sub	r3, r8
	sub	r3, r9

.r5g6b5_y:
	# First longword
	mov.w	@r4, r1
	mov	r2, r0
	mov.w	r1, @r5

	# Last longword; align to 4-byte boundaries for target
	mov.w	@(r0, r4), r1
	add	r3, r4
	mov.w	r1, @(r0, r5)
	add	r3, r5

	mov	r6, r10

.r5g6b5_x:
	# Copy longwords
	movua.l	@r4+, r0
	mov.l	r0, @r5

	dt	r10
	bf.s	.r5g6b5_x
	add	#4, r5

# -

	add	r8, r4
	dt	r7
	bf.s	.r5g6b5_y
	add	r9, r5

# -

	mov.l	@r15+, r10
	mov.l	@r15+, r9
	rts
	mov.l	@r15+, r8

# REGISTER ALLOCATION:
# r0: (tmp)
# r1: in_stride
# r2: out_stride
# r3: x counter
# ---
# r4: data
# r5: target
# r6: width
# r7: height
# ---
# r8: alpha
# ---
# @4:  in_stride
# @8:  out_stride
# @12: alpha

.align 4

_bopti_r5g6b5a:
	# Load alpha value
	mov.l	r8, @-r15
	mov.l	@(12, r15), r8

	# Load input and output strides
	mov.l	@(4, r15), r1
	mov.l	@(8, r15), r2

.r5g6b5a_y:
	mov	r6, r3

.r5g6b5a_x:
	mov.w	@r4+, r0
	cmp/eq	r0, r8
	bt	.r5g6b5a_alpha

	mov.w	r0, @r5

.r5g6b5a_alpha:
	dt	r3
	bf.s	.r5g6b5a_x
	add	#2, r5

# -

	add	r1, r4
	dt	r7
	bf.s	.r5g6b5a_y
	add	r2, r5

# -

	rts
	mov.l	@r15+, r8
