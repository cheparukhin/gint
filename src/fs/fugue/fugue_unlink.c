#include <gint/hardware.h>
#include <gint/bfile.h>
#include <gint/fs.h>
#include <errno.h>
#include <sys/stat.h>
#include "util.h"
#include "bfilecp.h"

int fugue_unlink(char const *path)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = fs_path_normalize_fc(path);
	if(!fcpath) {
		errno = ENOMEM;
		return -1;
	}

	int type, size, rc;
	rc = BFile_Ext_Stat(fcpath, &type, &size);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		free(fcpath);
		return -1;
	}
	if(bfile_type_to_mode_t(type) != S_IFREG) {
		errno = ENOTDIR;
		free(fcpath);
		return -1;
	}

#if GINT_OS_CP
	rc = BFile_Remove(path);
#else
	rc = BFile_Remove(fcpath);
#endif
	free(fcpath);

	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	return 0;
}
