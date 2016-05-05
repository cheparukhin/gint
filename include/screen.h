#ifndef	_SCREEN_H
#define	_SCREEN_H	1

/*
	screen_display()
	Displays contents on the full screen. Expects a 1024-byte buffer.

	@arg	vram	1024-byte video buffer.
*/
void screen_display(const void *vram);

#endif
