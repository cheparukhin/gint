//---
//	gint:core:mmu - MMU-related definitions
//---

#include <gint/mmu.h>
#include <gint/hardware.h>

//---
//	SH7705 TLB
//---

#ifdef FX9860G

/* tlb_addr() - get the P4 address of a TLB address entry */
GINLINE const tlb_addr_t *tlb_addr(uint way, uint E)
{
	uint32_t addr = 0xf2000000 | (E << 12) | (way << 8);
	return (void *)addr;
}

/* tlb_data() - get the P4 address of a TLB data entry */
GINLINE const tlb_data_t *tlb_data(uint way, uint E)
{
	uint32_t addr = 0xf3000000 | (E << 12) | (way << 8);
	return (void *)addr;
}

/* tlb_mapped_memory() - count amount of mapped memory */
void tlb_mapped_memory(uint32_t *p_rom, uint32_t *p_ram)
{
	uint32_t rom = 0, ram = 0;

	for(int way = 0; way < 4; way++)
	for(int E = 0; E < 32; E++)
	{
		const tlb_addr_t *addr = tlb_addr(way, E);
		const tlb_data_t *data = tlb_data(way, E);
		if(!addr->V || !data->V) continue;

		int size = data->SZ ? 4096 : 1024;
		uint32_t src;

		if(data->SZ) src = (((addr->VPN >> 2) | E) << 12);
		else src = (addr->VPN | (E << 2)) << 10;

		if(src >= 0x00300000 && src < 0x00380000) rom += size;
		if(src >= 0x08100000 && src < 0x08180000) ram += size;
	}

	if(p_rom) *p_rom = rom;
	if(p_ram) *p_ram = ram;

	gint[HWURAM] = ram;
}

#endif

//---
//	SH7305 Unified TLB
//---

/* utlb_addr() - get the P4 address of a UTLB address entry */
GINLINE const utlb_addr_t *utlb_addr(uint E)
{
	uint32_t addr = 0xf6000000 | ((E & 0x3f) << 8);
	return (void *)addr;
}

/* utlb_data() - get the P4 address of a UTLB data entry */
GINLINE const utlb_data_t *utlb_data(uint E)
{
	uint32_t addr = 0xf7000000 | ((E & 0x3f) << 8);
	return (void *)addr;
}

/* utlb_mapped_memory() - count amount of mapped memory */
void utlb_mapped_memory(uint32_t *p_rom, uint32_t *p_ram)
{
	uint32_t rom = 0, ram = 0;

	for(int E = 0; E < 64; E++)
	{
		const utlb_addr_t *addr = utlb_addr(E);
		const utlb_data_t *data = utlb_data(E);
		if(!addr->V || !data->V) continue;

		/* Magic formula to get the size without using an array */
		int sz = ((data->SZ1 << 1) | data->SZ2) << 3;
		int size = 1 << ((0x14100c0a >> sz) & 0xff);

		uint32_t src = addr->VPN << 10;
		if(src >= 0x00300000 && src < 0x00700000) rom += size;
		if(src >= 0x08100000 && src < 0x08180000) ram += size;
	}

	if(p_rom) *p_rom = rom;
	if(p_ram) *p_ram = ram;

	gint[HWURAM] = ram;
}
