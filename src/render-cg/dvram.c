#include <gint/display.h>

/* Put both VRAMs in the system stack! */
static uint16_t *main = (void *)0xac0f0000;
static uint16_t *scnd = (void *)0xac11b500;

/* Shared VRAM pointer, the one exposed by <gint/display.h> */
uint16_t *gint_vram = (void *)0xac0f0000;

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
