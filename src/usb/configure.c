#include <gint/usb.h>
#include "usb_private.h"
#include <stdlib.h>

//---
// Endpoint assignment
//---

/* Current configuration: list of interfaces and endpoint assignments */
static usb_interface_t const *conf_if[16];
static endpoint_t *conf_ep;

GCONSTRUCTOR static void usb_alloc(void)
{
	 conf_ep = malloc(32 * sizeof *conf_ep);
}

/* usb_configure_endpoint(): Get endpoint data for a concrete address */
endpoint_t *usb_configure_endpoint(int endpoint)
{
	/* Refuse access to endpoint 0, which is for the DCP */
	if(endpoint < 0 || ((endpoint & 0x0f) == 0)) return NULL;

	int dir = (endpoint & 0x80) ? 0x10 : 0;
	endpoint &= 0x7f;

	if(endpoint <= 0 || endpoint > 15) return NULL;
	return &conf_ep[dir + endpoint];
}

/* usb_configure_address(): Get the concrete endpoint address */
int usb_configure_address(usb_interface_t const *intf, int address)
{
	int dir  = address & 0x80;
	int base = address & 0x0f;

	for(int i = 0; i < 32; i++)
	{
		if(conf_ep[i].intf != intf) continue;
		if((conf_ep[i].dc->bEndpointAddress & 0x0f) != base) continue;
		return (i & 0x0f) | dir;
	}
	return -1;
}

//---
// Resource allocation
//---

/* is_pipe_used(): Determine whether a pipe is used by an endpoint */
static bool is_pipe_used(int pipe)
{
	for(int i = 0; i < 32; i++)
	{
		if(conf_ep[i].pipe == pipe) return true;
	}
	return false;
}

/* find_pipe(): Find an unused pipe for the specified type of transfer */
static int find_pipe(int type)
{
	int min=0, max=-1;

	/* Isochronous transfers: use pipes 1,2 */
	if(type == 1) min=1, max=2;
	/* Bulk transfers: try 3..5 first, leaving 1,2 for isochronous */
	if(type == 2) min=1, max=5;
	/* Interrupt transfers: use pipes 6..9 */
	if(type == 3) min=6, max=9;

	/* Start from the end to avoid using pipes 1,2 on bulk transfers */
	for(int pipe = max; pipe >= min; pipe--)
	{
		if(!is_pipe_used(pipe)) return pipe;
	}
	return -1;
}

/* usb_configure_solve(): Allocate resources for all activated interfaces */
int usb_configure_solve(usb_interface_t const **interfaces)
{
	/* Reset the previous configuration */
	for(int i = 0; i < 16; i++)
	{
		conf_if[i] = NULL;
	}
	for(int i = 0; i < 32; i++)
	{
		conf_ep[i].intf    = NULL;
		conf_ep[i].dc      = NULL;
		conf_ep[i].pipe    = 0;
		conf_ep[i].bufnmb  = 0;
		conf_ep[i].bufsize = 0;
	}

	/* Next interface number to assign */
	int next_interface = 0;
	/* Next endpoint to assign */
	int next_endpoint = 1;
	/* Next buffer position to assign for pipes 1..5 */
	int next_bufnmb = 8;

	for(int i = 0; interfaces[i]; i++)
	{
		if(i == 16) return USB_OPEN_TOO_MANY_INTERFACES;
		usb_interface_t const *intf = interfaces[i];

		conf_if[next_interface++] = intf;

		for(int k = 0; intf->dc[k]; k++)
		{
			uint8_t const *dc = intf->dc[k];
			if(dc[1] != USB_DC_ENDPOINT) continue;

			/* If the same endpoint with a different direction has
			   already been assigned, use that */
			int address = usb_configure_address(intf, dc[2]);
			if(address == -1)
			{
				if(next_endpoint >= 16)
					return USB_OPEN_TOO_MANY_ENDPOINTS;
				address = (next_endpoint++) | (dc[2] & 0x80);
			}

			int pipe = find_pipe(dc[3] & 0x03);
			if(pipe < 0) return USB_OPEN_TOO_MANY_ENDPOINTS;

			endpoint_t *ep = usb_configure_endpoint(address);
			ep->intf = intf;
			ep->dc = (void *)dc;
			ep->pipe = pipe;
			ep->bufnmb = 0;
			ep->bufsize = 0;

			/* Fixed areas */
			if(pipe >= 6)
			{
				ep->bufnmb = (pipe - 2);
				ep->bufsize = 1;
			}
		}

		for(int k = 0; intf->params[k].endpoint; k++)
		{
			usb_interface_endpoint_t *params = &intf->params[k];

			int a = usb_configure_address(intf, params->endpoint);
			endpoint_t *ep = usb_configure_endpoint(a);
			if(!ep) return USB_OPEN_INVALID_PARAMS;

			uint bufsize = params->buffer_size >> 6;
			if(params->buffer_size & 63
			   || bufsize <= 0
			   || bufsize > 0x20
			   || (ep->pipe >= 6 && bufsize != 1))
				return USB_OPEN_INVALID_PARAMS;
			if(ep->pipe >= 6) continue;

			if(next_bufnmb + bufsize > 0x100)
				return USB_OPEN_NOT_ENOUGH_MEMORY;

			ep->bufnmb = next_bufnmb;
			ep->bufsize = bufsize;
			next_bufnmb += bufsize;
		}
	}

	/* Check that all endpoints have memory assigned */
	for(int i = 0; i < 32; i++)
	{
		if(!conf_ep[i].intf) continue;
		if(conf_ep[i].bufsize == 0)
			return USB_OPEN_MISSING_DATA;
	}

	return 0;
}

/* usb_configure_log(): Print configuration results in the usb_log() */
void usb_configure_log(void)
{
	/* Log the final configuration */
	for(int i = 0; i < 16 && conf_if[i]; i++)
		usb_log("Interface #%d: %p\n", i, conf_if[i]);

	for(int i = 0; i < 32; i++)
	{
		if(!conf_ep[i].intf) continue;

		endpoint_t *ep = &conf_ep[i];
		usb_log("Endpoint %02x\n",
			(i & 15) + (i >= 16 ? 0x80 : 0));
		usb_log("  Interface %p address %02x\n",
			ep->intf, ep->dc->bEndpointAddress);
		usb_log("  Pipe %d (FIFO: %02x..%02x)\n",
			ep->pipe, ep->bufnmb, ep->bufnmb + ep->bufsize);
	}
}

/* usb_configure(): Load the generated configuration to the USB module */
void usb_configure(void)
{
	for(int i = 0; i < 32; i++)
	{
		if(!conf_ep[i].intf) continue;
		int address = (i & 0xf) + (i >= 16 ? 0x80 : 0);
		usb_pipe_configure(address, &conf_ep[i]);
		usb_pipe_clear(conf_ep[i].pipe);
	}
}

/* usb_configure_interfaces(): List configures interfaces */
usb_interface_t const * const *usb_configure_interfaces(void)
{
	return conf_if;
}

//---
// API for interfaces
//---

int usb_interface_pipe(usb_interface_t const *interface, int endpoint)
{
	int concrete_address = usb_configure_address(interface, endpoint);
	endpoint_t const *ep = usb_configure_endpoint(concrete_address);
	if(!ep) return -1;

	return ep->pipe;
}
