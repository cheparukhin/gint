#ifndef	_SETJMP_H
#define	_SETJMP_H	1

// There are 16 CPU registers that *must* be saved to ensure a basically
// safe jump.
typedef	unsigned int jmp_buf[16];



//---
//	Long jump functions.
//---

/*
	setjmp()
	Configures a jump by saving data to the given jump buffer.

	@arg	env	Empty jump buffer.
*/
int setjmp(jmp_buf env);

/*
	longjmp()
	Performs a long jump.

	@arg	env	Jump buffer configure with setjmp().
	@arg	value	setjmp() will return this integer after the jump.
*/
void longjmp(jmp_buf env, int value);

#endif	// _SETJMP_H
