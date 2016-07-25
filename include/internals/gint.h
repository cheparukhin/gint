#ifndef _INTERNALS_GINT_H
#define	_INTERNALS_GINT_H	1

//---
//	Exception code strings.
//---

extern const char *gint_str[];



//---
//	Register access.
//---

/*
	gint_spc()
	Returns the saved program counter, which is the last state of execution
	saved by interrupt processing.
*/
unsigned int gint_spc(void);

/*
	gint_ssr()
	Returns the saved status register, which is the last configuration
	saved by interrupt processing.
*/
unsigned int gint_ssr(void);

#endif	// _INTERNALS_GINT_H
