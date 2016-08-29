#include <internals/mmu.h>

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

/*
	mmu_pseudoTLBInit()
	We need the system to fill the TLB for us, so that it knows what
	happens to its contents. We don't want to edit the TLB ourselves, so
	we'll just read random data from every 4k-page in the rom area to have
	the system virtualize it entirely.
	The index system for TLB entries ensures this process will work for
	128-k or less add-ins. On SH7705 there is a limit, which is 384-k (it
	does probably not exist on SH7305) but there is no guarantee that the
	system will not go wild after 128-k.
*/
void mmu_pseudoTLBInit(void)
{
	extern unsigned int romdata;
	unsigned int address = 0x00300000;
	unsigned int x;

	while(address <= (unsigned int)&romdata)
	{
		x = *((volatile unsigned int *)address);
		address += 0x1000;
	}
}

#pragma GCC diagnostic pop
