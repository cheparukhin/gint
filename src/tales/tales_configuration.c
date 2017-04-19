#include <internals/tales.h>
#include <tales.h>

/*
	text_configure()
	Sets the font and mode to use for the following print operations.
*/
void text_configure(font_t *next_font, color_t next_operator)
{
	extern font_t gint_font_system;
	if(next_font) font = next_font;
	else font = &gint_font_system;

	operator = next_operator;
}
