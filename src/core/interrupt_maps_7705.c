#include <internals/interrupt_maps.h>
#include <stddef.h>

//---
//	Mapping hardware interrupt info to gint interrupts -- 7705 interrupts
//---

#define NO_EVENT (gint_interrupt_map_t){ .type = 0, .subtype = 0 }

static gint_interrupt_map_t map0[16] = {
	NO_EVENT,			{ exc_manual_reset, 0 },
	{ exc_tlb_invalid, 1 },		{ exc_tlb_invalid, 2 },
	{ exc_initial_page_write, 0 },	{ exc_tlb_protection_violation, 1 },
	{ exc_tlb_protection_violation, 2 }, { exc_address_error, 1 },
};

static gint_interrupt_map_t map1[16] = {
	{ exc_address_error, 2 },	NO_EVENT,
	NO_EVENT,			{ exc_trap, 0 },
	{ exc_illegal_instruction, 0 },	{ exc_illegal_slot, 0 },
	{ int_nmi, 0 },			{ exc_user_break, 0 },
};

static gint_interrupt_map_t map4[16] = {
	{ int_timer_underflow, 0 },	{ int_timer_underflow, 1 },
	{ int_timer_underflow, 2 },	{ int_timer_input_capture, 0 },
	{ int_rtc_alarm, 0 },		{ int_rtc_periodic, 0 },
	{ int_rtc_carry, 0 },		NO_EVENT,
};

static gint_interrupt_map_t *map[16] = {
	map0,	map1,	NULL,	NULL,	map4,	NULL,	NULL,	NULL,
	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,
};

/*
	gint_map()
	Maps an event code and a VBR offset to an exception/interrupt type and
	subtype.
*/
gint_interrupt_map_t gint_map_7705(uint32_t event_code, uint32_t offset)
{
	// Handling TLB misses (only two events).
	if(offset == 0x400) switch(event_code)
	{
	case 0x040:
		return (gint_interrupt_map_t) {
			.type = exc_tlb_miss,
			.subtype = 1,
		};
	case 0x060:
		return (gint_interrupt_map_t) {
			.type = exc_tlb_miss,
			.subtype = 2,
		};
	default:
		return NO_EVENT;
	}

	// Handling general exceptions and interrupts.
	int ctgy = event_code >> 8;
	int event = (event_code & 0xff) >> 5;

	return map[ctgy] ? map[ctgy][event] : NO_EVENT;
}
