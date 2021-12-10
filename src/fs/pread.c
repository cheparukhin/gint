#include <unistd.h>
#include "util.h"

ssize_t pread(int fd, void *buf, size_t size, off_t offset)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	/* Thanks to the extra argument to BFile_Read(), we can perform this
	   call without knowing the current offset, even on G-III models */
	int rc = BFile_Read(fd, buf, size, offset);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	BFile_Seek(fd, -size);
	return rc;
}
