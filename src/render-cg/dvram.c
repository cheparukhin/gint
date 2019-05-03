#define GINT_NEED_VRAM
#include <gint/display.h>

/* Put both VRAMs in the system stack! */
static uint16_t *main = (void *)0xac0f0000;
static uint16_t *scnd = (void *)0xac11b500;

/* Shared VRAM pointer, the one exposed by GINT_NEED_VRAM */
uint16_t *vram = (void *)0xac0f0000;

/* dvram() - Control video RAM address and triple buffering */
void dvram(uint16_t *new_main, uint16_t *new_secondary)
{
	main = new_main;
	scnd = new_secondary;
}
