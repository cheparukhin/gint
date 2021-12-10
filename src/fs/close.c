#include <unistd.h>
#include "util.h"

int close(int fd)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	int err = BFile_Close(fd);
	if(err < 0) {
		errno = bfile_error_to_errno(err);
		return -1;
	}
	return 0;
}
