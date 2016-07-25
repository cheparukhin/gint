#include <internals/tales.h>
#include <tales.h>

/*
	text_configure()
	Sets the font and mode to use for the following print operations.
*/
void text_configure(struct Font *next_font)
{
	font = next_font;
}

/*
	text_configure_default()
	Configures tales with the default font (which is part of gint).
*/
void text_configure_default(void)
{
	extern Font gint_font_system_start;
	text_configure(&gint_font_system_start);
}
