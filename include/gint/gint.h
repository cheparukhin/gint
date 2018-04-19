//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#include <defs/attributes.h>
#include <defs/types.h>

/* gint_version() - get the library version number
   Returns gint's running version number, which is made of four fields:

      31           24 23   20 19   16 15                            0
     +---------------+---------------+---------------+---------------+
     |    channel    | major | minor |             build             |
     +---------------+---------------+---------------+---------------+

   The first field is a letter indicating the type of version ('a'lpha, 'b'eta,
   'r'elease, 'd'ev, etc). The second and third field are the version number on
   the form "major.minor". The last field is the build number for this version.
   The build number uniquely identifies a binary version of the library.

   For instance, 0x72100053 translates as "release 1.0 build 83". */
HDRFUNC uint32_t gint_version(void)
{
	extern char GINT_VERSION;
	return (uint32_t)&GINT_VERSION;
}

//---
//	Library management
//---

/* gint_install() - install and start gint
   This function installs event handlers, masks interrupts and switches VBR.
   Unless you are doing experimental runtime switching and you know how this
   function is implemented, you should not call it. */
void gint_install(void);

/* gint_unload() - unload gint and give back control to the system
   This function restores the runtime environment saved by gint_install(). It
   is only called when the add-in terminates. To temporarily leave gint during
   execution, use gint_pause(). When possible, use syscalls without leaving
   gint for better performance. */
void gint_unload(void);

/* gint_pause() - return to main menu, with possibility of coming back

   This function safely invokes the calculator's main menu by unloading gint.
   If the user selects the gint application again in the menu, this function
   reloads gint and returns. Otherwise, the add-in is fully unloaded by the
   system and the application terminates.

   This function is typically called when the [MENU] key is pressed during a
   getkey() call. */
void gint_pause(void);

#endif /* GINT_GINT */
