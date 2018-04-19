//---
//	gint:core:mmu - MMU-related definitions
//---

#include <core/mmu.h>

/* utlb_addr() - get the P4 address of a UTLB address entry */
INLINE const utlb_addr_t *utlb_addr(uint E)
{
	uint32_t addr = 0xf6000000 | ((E & 0x3f) << 8);
	return (void *)addr;
}

/* utlb_data() - get the P4 address of a UTLB data entry */
INLINE const utlb_data_t *utlb_data(uint E)
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

		int sz = ((data->SZ1 << 1) | data->SZ2) << 3;
		int size = 1 << ((0x14100c0a >> sz) & 0xff);

		uint32_t src = addr->VPN << 10;
		if(src >= 0x00300000 && src < 0x00380000) rom += size;
		if(src >= 0x08100000 && src < 0x08180000) ram += size;
	}

	if(p_rom) *p_rom = rom;
	if(p_ram) *p_ram = ram;
}
