#include <unistd.h>
#include "util.h"

int unlink(char const *path)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = utf8_to_fc_alloc(path, u"\\\\fls0\\");
	if(!fcpath) {
		errno = ENOMEM;
		return -1;
	}

	int err = BFile_Remove(fcpath);
	if(err < 0) {
		errno = bfile_error_to_errno(err);
		return -1;
	}

	return 0;
}
