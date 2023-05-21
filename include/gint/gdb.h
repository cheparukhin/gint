//---
// gint:gdb - GDB remote serial protocol
//---

#ifndef GINT_GDB
#define GINT_GDB

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes for GDB functions */
enum {
	GDB_NO_INTERFACE = -1,
	GDB_ALREADY_STARTED = -2,
	GDB_USB_ERROR = -3,
};

/* gdb_start(): Start the GDB remote serial protocol server

   This function will start the GDB remote serial protocol implementation and
   block until the program is resumed from the connected debugger.
   It currently only supports USB communication and will fail if USB is already
   in use.*/
int gdb_start(void);

#ifdef __cplusplus
}
#endif

#endif /* GINT_GDB */
