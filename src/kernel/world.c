#include <gint/drivers.h>
#include <gint/cpu.h>
#include <gint/gint.h>
#include <gint/defs/call.h>
#include <gint/std/stdlib.h>

//---
// World buffer
//---

gint_world_t gint_world_alloc(void)
{
	size_t header_size = gint_driver_count() * sizeof(void *);
	size_t data_size = 0;

	for(int i = 0; i < gint_driver_count(); i++)
		data_size += (gint_drivers[i].state_size + 3) & ~3;

	void *buffer = malloc(header_size + data_size);
	if(!buffer) return NULL;

	gint_world_t world = buffer;
	buffer += header_size;

	for(int i = 0; i < gint_driver_count(); i++)
	{
		world[i] = buffer;
		buffer += (gint_drivers[i].state_size + 3) & ~3;
	}

	return world;
}

void gint_world_free(gint_world_t world)
{
	free(world);
}

//---
// World switch with driver state saves
//---

void gint_world_switch_in(gint_world_t world_os, gint_world_t world_addin)
{
	/* Unbind from the OS driver and complete foreign asynchronous tasks */
	for(int i = gint_driver_count() - 1; i >= 0; i--)
	{
		gint_driver_t *d = &gint_drivers[i];
		if(d->funbind) d->funbind();
	}

	cpu_atomic_start();

	for(int i = 0; i < gint_driver_count(); i++)
	{
		gint_driver_t *d = &gint_drivers[i];
		uint8_t *f = &gint_driver_flags[i];

		bool foreign_powered = (!d->hpowered || d->hpowered());
		if(foreign_powered)
			*f |= GINT_DRV_FOREIGN_POWERED;
		else
			*f &= ~GINT_DRV_FOREIGN_POWERED;

		/* Power the device if it was unpowered previously */
		if(!foreign_powered && d->hpoweron) d->hpoweron();

		/* For non-shared devices, save previous device state and
		   consider restoring the preserved one */
		if(!(*f & GINT_DRV_SHARED))
		{
			if(d->hsave)
				d->hsave(world_os[i]);
			if(!(*f & GINT_DRV_CLEAN) && d->hrestore)
				d->hrestore(world_addin[i]);
		}

		/* Bind the driver, configure if needed. Note that we either
		   configure or restore the new world's state, not both */
		if(d->bind) d->bind();

		if(*f & GINT_DRV_CLEAN)
		{
			if(d->configure) d->configure();
			*f &= ~GINT_DRV_CLEAN;
		}
	}

	cpu_atomic_end();
}

void gint_world_switch_out(gint_world_t world_addin, gint_world_t world_os)
{
	for(int i = gint_driver_count() - 1; i >= 0; i--)
	{
		gint_driver_t *d = &gint_drivers[i];
		if(d->unbind) d->unbind();
	}

	cpu_atomic_start();

	for(int i = gint_driver_count() - 1; i >= 0; i--)
	{
		gint_driver_t *d = &gint_drivers[i];
		uint8_t *f = &gint_driver_flags[i];

		/* For non-shared devices, save previous device state and
		   consider restoring the preserved one */
		if(!(*f & GINT_DRV_SHARED))
		{
			if(d->hsave) d->hsave(world_addin[i]);
			if(d->hrestore) d->hrestore(world_os[i]);
		}

		/* Restore the power state of the device */
		if(!(*f & GINT_DRV_FOREIGN_POWERED) && d->hpoweroff)
			d->hpoweroff();
	}

	cpu_atomic_end();
}

int gint_world_switch(gint_call_t call)
{
	gint_world_switch_out(gint_world_addin, gint_world_os);
	int rc = call.function ? gint_call(call) : 0;
	gint_world_switch_in(gint_world_os, gint_world_addin);
	return rc;
}

void gint_switch(void (*function)(void))
{
	gint_world_switch(GINT_CALL(function));
}
