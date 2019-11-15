
.global _bopti_r5g6b5
.global _bopti_r5g6b5a
.global _bopti_p8
.global _bopti_p4

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

	# Input and output strides. Add ending alignment because there is no
	# corresponding increment in the y-loop.
	mov.l	@(12, r15), r8
	mov.l	@(16, r15), r9
	mov	r5, r0
	shll	r6
	add	r6 ,r0
	and	#2, r0
	add	r0, r8
	add	r0, r9

	# Number of longword operations per row
	sub	r3, r6
	shlr2	r6

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
# r8: palette
# r9: alpha
# ---
# @8:  in_stride
# @12: out_stride
# @16: palette
# @20: alpha

.align 4

_bopti_p8:
	# Load palette and in/out strides
	mov.l	r8, @-r15
	mov.l	r9, @-r15
	mov.l	@(16, r15), r8
	mov.l	@(8, r15), r1
	mov.l	@(12, r15), r2

	# Load alpha value
	mov.l	@(20, r15), r9

.p8_y:
	mov	r6, r3

.p8_x:
	mov.b	@r4+, r0
	extu.b	r0, r0
	cmp/eq	r0, r9
	bt	.p8_alpha

	# Pass pixel through palette
	shll	r0
	mov.w	@(r0, r8), r0
	mov.w	r0, @r5

.p8_alpha:
	dt	r3
	bf.s	.p8_x
	add	#2, r5

# -

	add	r1, r4
	dt	r7
	bf.s	.p8_y
	add	r2, r5

# -

	mov.l	@r15+, r9
	rts
	mov.l	@r15+, r8

# REGISTER ALLOCATION:
# r0: (tmp)
# r1: in_stride (in pixels)
# r2: out_stride
# r3: x counter
# ---
# r4: data
# r5: target
# r6: width
# r7: height
# ---
# r8:  palette
# r9:  number of pixels of offset into data (r4)
# r10: alpha
# ---
# @12: in_stride
# @16: out_stride
# @20: palette
# @24: alpha
# @28: initial offset (in pixels)

.align 4

_bopti_p4:
	mov.l	r8, @-r15
	mov.l	r9, @-r15
	mov.l	r10, @-r15

	# Load palette and in/out strides
	mov.l	@(20, r15), r8
	mov.l	@(12, r15), r1
	mov.l	@(16, r15), r2

	# Load initial offset
	mov.l	@(28, r15), r9

	# Load alpha value
	mov.l	@(24, r15), r10
	shll	r10

.p4_y:
	mov	r6, r3

.p4_x:

	# Load 4 bits from offset r9 (in pixels) within image data (r4). Note
	# that [shlr] puts bit 0 of the shifted register in T.
	mov	r9, r0
	add	#1, r9
	shlr	r0
	bt.s	.p4_x_unaligned
	mov.b	@(r0, r4), r0

.p4_x_aligned:

	# Load 4 bits from the higher half of @r4 and use them to index the
	# palette. Since the palette has two-byte entries, we need the color
	# bits to be in position 000xxxx0.
	shlr2	r0
	shlr	r0
	and	#0x1e, r0

	cmp/eq	r0, r10
	bt	.p4_alpha

	# Pass pixel through palette
	mov.w	@(r0, r8), r0
	bra	.p4_alpha
	mov.w	r0, @r5

.p4_x_unaligned:

	# Load 4 bits from the lower half of @r4 into position 000xxxx0.
	shll	r0
	and	#0x1e, r0

	cmp/eq	r0, r10
	bt	.p4_alpha

	# Pass pixel through palette
	mov.w	@(r0, r8), r0
	mov.w	r0, @r5

.p4_alpha:

	dt	r3
	bf.s	.p4_x
	add	#2, r5

.p4_y_end:

	add	r1, r9
	dt	r7
	bf.s	.p4_y
	add	r2, r5

# -

	mov.l	@r15+, r10
	mov.l	@r15+, r9
	rts
	mov.l	@r15+, r8
