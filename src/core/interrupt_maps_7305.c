#include <internals/interrupt_maps.h>
#include <stddef.h>

//---
//	Mapping hardware interrupts info to gint interrupts -- 7305 interrupts
//---

#define NO_EVENT (gint_interrupt_map_t){ .type = 0, .subtype = 0 }

static gint_interrupt_map_t map0[16] = {
	{ exc_poweron_reset, 0 },	{ exc_manual_reset, 0 },
	{ exc_tlb_miss, 1 },		{ exc_tlb_miss, 2 },
	{ exc_initial_page_write, 0 },	{ exc_tlb_protection_violation, 1 },
	{ exc_tlb_protection_violation, 2 }, { exc_address_error, 1 },
};

static gint_interrupt_map_t map1[16] = {
	{ exc_address_error, 2 },	NO_EVENT, /* FPU exception */
	{ exc_tlb_multihit, 0 },	{ exc_trap, 0 },
	{ exc_illegal_instruction, 0 },	{ exc_illegal_slot, 0 },
	{ int_nmi, 0 },			{ exc_user_break, 0 },
};

static gint_interrupt_map_t map4[16] = {
	{ int_timer_underflow, 0 },	{ int_timer_underflow, 1 },
	{ int_timer_underflow, 2 },	NO_EVENT,
	NO_EVENT,			NO_EVENT,
	NO_EVENT,			NO_EVENT, /* SDHI1 [SDHII0] */
};

static gint_interrupt_map_t mapa[16] = {
	NO_EVENT,			NO_EVENT, /* USB0 [USI0] */
	NO_EVENT, /* USB0 [USI1] */	NO_EVENT,
	{ int_rtc_alarm, 0 },		{ int_rtc_periodic, 0 },
	{ int_rtc_carry, 0 },		NO_EVENT,
};

static gint_interrupt_map_t *map[16] = {
	map0,	map1,	NULL,	NULL,	map4,	NULL,	NULL,	NULL,
	NULL,	NULL,	mapa,	NULL,	NULL,	NULL,	NULL,	NULL,
};

/*
	gint_map()
	Maps an event code to an exception/interrupt type and subtype.
*/
gint_interrupt_map_t gint_map_7305(uint32_t event_code)
{
	int ctgy = event_code >> 8;
	int event = (event_code & 0xff) >> 5;

	return map[ctgy] ? map[ctgy][event] : NO_EVENT;
}
