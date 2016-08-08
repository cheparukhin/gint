#include <time.h>

/*
	ctime()
	Converts calendar time to string representation on the form
	"Wed Jun 30 21:49:08 1993\n". The returned string is statically
	allocated and may be overwritten by any subsequent call to a time
	function.
*/
char *ctime(const time_t *timer)
{
	return asctime(gmtime(timer));
}
