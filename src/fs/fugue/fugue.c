#include <gint/fs.h>
#include <gint/hardware.h>
#include <gint/bfile.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"

ssize_t fugue_read(void *data, void *buf, size_t size)
{
	int fugue_fd = (int)data;

	int rc = BFile_Read(fugue_fd, buf, size, -1);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	return size;
}

ssize_t fugue_pread(void *data, void *buf, size_t size, off_t offset)
{
	int fugue_fd = (int)data;

	/* Thanks to the extra argument to BFile_Read(), we can perform this
	   call without knowing the current offset, even on G-III models */
	int rc = BFile_Read(fugue_fd, buf, size, offset);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	BFile_Seek(fugue_fd, -size);
	return rc;
}

ssize_t fugue_write(void *data, const void *buf, size_t size)
{
	int fugue_fd = (int)data;

	int rc = BFile_Write(fugue_fd, buf, size);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	return rc;
}

off_t fugue_lseek(void *data, off_t offset, int whence)
{
	int fugue_fd = (int)data;

	/* On Graph 35+E II, there is no documented way to know the offset. */
	if(gint[HWCALC] == HWCALC_G35PE2 && whence == SEEK_CUR) {
		errno = ENOTSUP;
		return (off_t)-1;
	}

	if(whence == SEEK_CUR)
		offset += BFile_GetPos(fugue_fd);
	else if(whence == SEEK_END)
		offset += BFile_Size(fugue_fd);

	int rc = BFile_Seek(fugue_fd, offset);

	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}

	/* rc is the amount of space left in the file (including pre-allocated
	   space), so instead just return offset directly */
	return offset;
}

int fugue_close(void *data)
{
	int fugue_fd = (int)data;

	int rc = BFile_Close(fugue_fd);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	return 0;
}

const fs_descriptor_type_t fugue_descriptor_type = {
	.read   = fugue_read,
	.pread  = fugue_pread,
	.write  = fugue_write,
	.lseek  = fugue_lseek,
	.close  = fugue_close,
};
