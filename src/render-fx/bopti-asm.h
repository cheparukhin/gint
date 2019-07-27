//---
//	gint:render-fx:bopti-asm - Assembler drawing routines for bopti
//---

#ifndef GINT_RENDERFX_BOPTIASM
#define GINT_RENDERFX_BOPTIASM

/* pair_t: A pair of consecutive VRAM longwords */
typedef struct {
	uint32_t l;
	uint32_t r;
} pair_t;

/* quadr_t: Two pairs for light and gray VRAMs */
typedef struct {
	uint32_t l1;
	uint32_t r1;
	uint32_t l2;
	uint32_t r2;
} quadr_t;

/* Signature of mono rendering functions */
typedef pair_t asm_mono_t(pair_t p,  void **layer, uint32_t *masks, int x);
/* Signature of gray rendering functions */
typedef void asm_gray_t(quadr_t q, void **layer, uint32_t *masks, int x,
	quadr_t *ret);

/* Each of the following rendering functions:
   1. Takes VRAM data for two longword positions of the screen.
   2. Reads data for one longword position of the image from *layer. This
      consists in n longwords where n is the number of layers in the image.
   3. Increments *layer by 4*n.
   4. Shifts the image data and apply it to the VRAM positions in accordance
      with the two masks given in the masks argument. */

/* bopti_asm_mono(): Rendering function for the "mono" profile */
extern asm_mono_t bopti_asm_mono;
/* bopti_asm_mono_alpha(): Rendering function for the "mono alpha" profile */
extern asm_mono_t bopti_asm_mono_alpha;

/* bopti_gasm_mono(): "mono" profile on gray VRAMs */
extern asm_gray_t bopti_gasm_mono;
/* bopti_gasm_mono_alpha(): "mono alpha" profile on gray VRAMs */
extern asm_gray_t bopti_gasm_mono_alpha;
/* bopti_asm_gray(): Rendering function for the "gray" profile */
extern asm_gray_t bopti_gasm_gray;
/* bpoti_asm_gray_alpha(): Rendering function for the "gray alpha" profile */
extern asm_gray_t bopti_gasm_gray_alpha;

#endif /* GINT_RENDERFX_BOPTIASM */
