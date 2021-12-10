#include <unistd.h>
#include "util.h"

ssize_t write(int fd, const void *buf, size_t size)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	int rc = BFile_Write(fd, buf, size);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	return rc;
}
