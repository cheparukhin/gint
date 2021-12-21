#include <gint/hardware.h>
#include <gint/bfile.h>
#include <errno.h>
#include "util.h"

int fugue_unlink(char const *path)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = utf8_to_fc_alloc(u"\\\\fls0\\", path, NULL);
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

int fugue_rmdir(char const *path)
{
	return fugue_unlink(path);
}
