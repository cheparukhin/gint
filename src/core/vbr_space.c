#include <internals/gint.h>
#include <internals/interrupt_maps.h>
#include <mpu.h>

// Compiler optimization of the interrupt handlers seems to cause crashes at
// some point. Some research showed that illegal slot exceptions were raised on
// rte; lds.l @r15+, mach, even though it's a legal slot. For now I just turn
// off optimization until I figure out where the true problem is.
#pragma GCC push_options
#pragma GCC optimize("O0")

//---
//	VBR space
//---

#ifdef	GINT_DIAGNOSTICS
static void register_interrupt(int offset)
{
	volatile gint_diagnostics_t *dg = gint_diagnostics();
	volatile uint32_t *expevt, *intevt, *tea;
	uint32_t event_code, spc, ssr;

	// Getting the addresses of some registers.
	expevt	= gint_reg(register_expevt);
	intevt	= gint_reg(register_intevt);
	tea	= gint_reg(register_tea);

	// Adding an entry in the event history.
	event_code = (offset == 0x600) ? (*intevt) : (*expevt);
	size_t len = sizeof dg->except_vect;
	dg->except_vect[dg->excepts++] = event_code >> 4;
	if(dg->excepts >= len) dg->excepts -= len;

	// Updating some fields in the diagnostic record.
	__asm__("stc spc, %0" : "=r"(spc));
	__asm__("stc ssr, %0" : "=r"(ssr));
	dg->spc = spc;
	dg->ssr = ssr;
	dg->expevt = event_code;
	dg->tea = *tea;
}
#endif

/*
	gint_exc()
	Handles exceptions.
*/
__attribute__((section(".gint.exc"), interrupt_handler))
void gint_exc(void)
{
	#ifdef	GINT_DIAGNOSTICS
	register_interrupt(0x100);
	#endif

	uint32_t event = *(uint32_t *)gint_reg(register_expevt);
	gint_interrupt_map_t map;
	map = isSH3() ? gint_map_7705(event, 0x100) : gint_map_7305(event);

	gint_invoke(map.type, map.subtype);
}

/*
	gint_tlb()
	Handles TLB misses.
*/
__attribute__((section(".gint.tlb"), interrupt_handler))
void gint_tlb(void)
{
	#ifdef	GINT_DIAGNOSTICS
	register_interrupt(0x400);
	#endif

	uint32_t event = *(uint32_t *)gint_reg(register_expevt);
	gint_interrupt_map_t map;
	map = isSH3() ? gint_map_7705(event, 0x400) : gint_map_7305(event);

	gint_invoke(map.type, map.subtype);
}

/*
	gint_int()
	Handles interrupts.
*/
__attribute__((section(".gint.int"), interrupt_handler))
void gint_int(void)
{
	#ifdef	GINT_DIAGNOSTICS
	register_interrupt(0x600);
	#endif

	uint32_t event = *(uint32_t *)gint_reg(register_intevt);
	gint_interrupt_map_t map;
	map = isSH3() ? gint_map_7705(event, 0x600) : gint_map_7305(event);

	gint_invoke(map.type, map.subtype);
}

#pragma GCC pop_options
