#include <unistd.h>
#include "util.h"

ssize_t pwrite(int fd, const void *buf, size_t size, off_t offset)
{
	ENOTSUP_IF_NOT_FUGUE((off_t)-1);

	off_t current = lseek(fd, 0, SEEK_CUR);

	if(current == (off_t)-1)
		return (ssize_t)-1;

	ssize_t rc = -1;

	if(lseek(fd, 0, offset) == (off_t)-1)
		goto end;

	rc = BFile_Write(fd, buf, size);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		goto end;
	}

end:
	/* At the end, always try to restore the current position */
	lseek(fd, current, SEEK_CUR);
	return rc;
}
