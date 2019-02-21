//---
//	gint:render-fx:topti-asm - Assembler drawing routines for topti
//---

#ifndef GINT_RENDERFX_TOPTIASM
#define GINT_RENDERFX_TOPTIASM

/* Signature of text rendering functions (which do not render text but really
   just blend a column of operators onto the VRAM */
typedef void asm_text_t(uint32_t *v1, uint32_t *v2, uint32_t *op, int height);

/* One rendering function per color */
extern asm_text_t *topti_asm_text[8];

#endif /* GINT_RENDERFX_TOPTIASM */
