//---
//	gint:exc - Exception handling
//
//	This small module is used to display exceptions and configure when the
//	exception handler displays these messages. This is for advanced users
//	only!
//---

#ifndef GINT_EXC
#define GINT_EXC

/* gint_exc(): Exception handler fatal error message
   Displays a full-screen fatal error message from an exception event code.
   Some custom event codes are also used for library failure. This function is
   typically called by exception handlers and returns after a dupdate_noint().
   You probably want to soft-lock or quit the add-in after calling it. */
void gint_exc(uint32_t code);

/* TODO: Add functions to disable fatal errors for some exceptions */

#endif /* GINT_EXC */
