#include <unistd.h>
#include "util.h"

ssize_t read(int fd, void *buf, size_t size)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	int err = BFile_Read(fd, buf, size, -1);
	if(err < 0) {
		errno = bfile_error_to_errno(err);
		return -1;
	}
	return size;
}
