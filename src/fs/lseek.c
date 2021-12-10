#include <unistd.h>
#include "util.h"

off_t lseek(int fd, off_t offset, int whence)
{
	ENOTSUP_IF_NOT_FUGUE((off_t)-1);

	if(whence != SEEK_SET && whence != SEEK_CUR && whence != SEEK_END) {
		errno = EINVAL;
		return (off_t)-1;
	}

	/* On Graph 35+E II, there is no documented way to know the offset. */
	if(gint[HWCALC] == HWCALC_G35PE2 && whence == SEEK_CUR) {
		errno = ENOTSUP;
		return (off_t)-1;
	}

	off_t destination;
	if(whence == SEEK_SET)
		destination = offset;
	else if(whence == SEEK_CUR)
		destination = BFile_GetPos(fd) + offset;
	else if(whence == SEEK_END)
		destination = BFile_Size(fd) - offset;

	int rc = BFile_Seek(fd, destination);

	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}

	/* rc is the amount of space left in the file (including pre-allocated
	   space), so instead just return destination directly */
	return (off_t)destination;
}
