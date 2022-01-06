#include <gint/fs.h>
#include <gint/hardware.h>
#include <gint/bfile.h>
#include <gint/defs/util.h>
#include <fcntl.h>
#include <errno.h>
#include "fugue.h"
#include "util.h"

ssize_t fugue_read(void *data0, void *buf, size_t size)
{
	fugue_fd_t *data = data0;
	int fugue_fd = data->fd;

	/* Fugue allows to read past EOF up to the end of the sector */
	int filesize = BFile_Size(fugue_fd);
	if(data->pos + (int)size > filesize)
		size = filesize - data->pos;

	int rc = BFile_Read(fugue_fd, buf, size, -1);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	data->pos += rc;
	return size;
}

ssize_t fugue_write(void *data0, const void *buf, size_t size)
{
	fugue_fd_t *data = data0;
	int fugue_fd = data->fd;

	int rc = BFile_Write(fugue_fd, buf, size);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	data->pos += rc;
	return rc;
}

off_t fugue_lseek(void *data0, off_t offset, int whence)
{
	fugue_fd_t *data = data0;
	int fugue_fd = data->fd;

	int filesize = BFile_Size(fugue_fd);

	if(whence == SEEK_CUR)
		offset += data->pos;
	else if(whence == SEEK_END)
		offset += filesize;

	/* BFile_Seek() clamps to the file size, but still returns the argument
	   when it does... */
	offset = min(offset, filesize);

	int rc = BFile_Seek(fugue_fd, offset);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	data->pos = offset;

	/* rc is the amount of space left in the file (including pre-allocated
	   space), so instead just return offset directly */
	return offset;
}

int fugue_close(void *data0)
{
	fugue_fd_t *data = data0;
	int fugue_fd = data->fd;

	int rc = BFile_Close(fugue_fd);
	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}
	free(data);
	return 0;
}

const fs_descriptor_type_t fugue_descriptor_type = {
	.read   = fugue_read,
	.write  = fugue_write,
	.lseek  = fugue_lseek,
	.close  = fugue_close,
};
