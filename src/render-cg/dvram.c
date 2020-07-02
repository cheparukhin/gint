#include <gint/display.h>

#ifdef GINT_USER_VRAM
/* We want to put the VRAM in the user section, however we can't use the
   virtualized P0 address as this will mess up the DMA. As the static RAM is
   always fully mapped at a fixed place, we can use the target P1 address. We
   just need to allocate the space for the linker. This special section ensures
   that the first address of the user stack will be used */
GALIGNED(4) GSECTION(".bss.vram") int8_t _gint_vram_buffers[396*224*2];

/* In this case, we can define pointers to our user stack directly, these will
   be the physical address associated with _vram_buffers */
static uint16_t *main = (void *)0xac161400;
static uint16_t *scnd = (void *)0xac18c900;

/* Shared VRAM pointer, the one exposed by <gint/display.h> */
uint16_t *gint_vram = (void *)0xac161400;

#else

/* Otherwise, just put both VRAMs in the system stack! */
static uint16_t *main = (void *)0xac0f0000;
static uint16_t *scnd = (void *)0xac11b500;
uint16_t *gint_vram = (void *)0xac0f0000;
#endif

/* On Prizm: should be: 0xa80f0000 and 0xa811b500 */


/* dsetvram() - Control video RAM address and triple buffering */
void dsetvram(uint16_t *new_main, uint16_t *new_secondary)
{
	if(gint_vram == main) gint_vram = new_main;
	else if(gint_vram == scnd) gint_vram = new_secondary;

	main = new_main;
	scnd = new_secondary;
}

/* dgetvram() - Get VRAM addresses */
void dgetvram(uint16_t **ptr_main, uint16_t **ptr_scnd)
{
	if(ptr_main) *ptr_main = main;
	if(ptr_scnd) *ptr_scnd = scnd;
}

/* dvram_switch() - Triple buffering switch
   This function is not public, it is used only by dupdate(). */
void dvram_switch(void)
{
	if(gint_vram == main) gint_vram = scnd;
	else gint_vram = main;
}
