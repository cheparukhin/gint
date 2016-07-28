#include <internals/tales.h>
#include <tales.h>

/*
	text_configure()
	Sets the font and mode to use for the following print operations.
*/
void text_configure(struct Font *next_font, enum Color next_color)
{
	extern Font gint_font_system_start;
	if(next_font) font = next_font;
	else font = &gint_font_system_start;

	color = next_color;
}
