#include <gint/hardware.h>
#include <gint/bfile.h>
#include <gint/fs.h>
#include <errno.h>
#include "util.h"

int fugue_unlink(char const *path)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = fs_path_normalize_fc(path);
	if(!fcpath) {
		errno = ENOMEM;
		return -1;
	}

	int err = BFile_Remove(fcpath);
	if(err < 0) {
		errno = bfile_error_to_errno(err);
		free(fcpath);
		return -1;
	}

	free(fcpath);
	return 0;
}

int fugue_rmdir(char const *path)
{
	return fugue_unlink(path);
}
