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
		destination = BFile_GetPos(fugue_fd) + offset;
	else if(whence == SEEK_END)
		destination = BFile_Size(fugue_fd) + offset;

	int rc = BFile_Seek(fugue_fd, destination);

	if(rc < 0) {
		errno = bfile_error_to_errno(rc);
		return -1;
	}

	/* rc is the amount of space left in the file (including pre-allocated
	   space), so instead just return destination directly */
	return (off_t)destination;
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

static const fs_descriptor_type_t fugue_descriptor_type = {
	.read   = fugue_read,
	.pread  = fugue_pread,
	.write  = fugue_write,
	.lseek  = fugue_lseek,
	.close  = fugue_close,
};

int fugue_open(char const *path, int flags, GUNUSED mode_t mode)
{
	if(gint[HWFS] != HWFS_FUGUE) {
		errno = ENOTSUP;
		return -1;
	}

	uint16_t *fcpath = utf8_to_fc_alloc(path, u"\\\\fls0\\");
	int fugue_fd, err, rc = -1;

	if(!fcpath) {
		errno = ENOMEM;
		return -1;
	}

	/* Open mode */
	int bfile_mode = BFile_ReadOnly;
	if(flags & O_WRONLY)
		bfile_mode = BFile_WriteOnly;
	else if(flags & O_RDWR)
		bfile_mode = BFile_ReadWrite;

	/* Exclusive creation requires the file to be created by the call */
	bool excl = (flags & O_EXCL) && (flags & O_CREAT);
	/* Truncation requires the file to be removed/recreated */
	bool trunc = (flags & O_TRUNC) && (flags & O_CREAT);

	/* Try and open the file normally, unless O_TRUNC is specified without
	   O_EXCL, in which case we simply delete and recreate the file. */
	fugue_fd = BFile_EntryNotFound;
	if(excl || !trunc)
		fugue_fd = BFile_Open(fcpath, bfile_mode);

	/* If the file exists and O_EXCL was requested, fail. */
	if(fugue_fd >= 0 && excl) {
		BFile_Close(fugue_fd);
		errno = EEXIST;
		return -1;
	}

	/* If O_TRUNC is requested and either the file exists or O_CREAT is
	   set, temporarily remove it. */
	if((flags & O_TRUNC) && (fugue_fd >= 0 || (flags & O_CREAT))) {
		if(fugue_fd >= 0) BFile_Close(fugue_fd);
		BFile_Remove(fcpath);
		fugue_fd = BFile_EntryNotFound;
	}

	/* If the file does not exist and O_CREAT is set, create it */
	if((flags & O_CREAT) && ((flags & O_TRUNC) || fugue_fd < 0)) {
		int size = 0;
		err = BFile_Create(fcpath, BFile_File, &size);
		if(err < 0) {
			errno = bfile_error_to_errno(err);
			goto end;
		}
		fugue_fd = BFile_Open(fcpath, bfile_mode);
	}

	if(fugue_fd < 0) {
		errno = bfile_error_to_errno(fugue_fd);
		goto end;
	}

	/* If O_APPEND is set, move to the end of the file */
	if((flags & O_APPEND))
		BFile_Seek(fugue_fd, BFile_Size(fugue_fd));

	/* Return the now-open file descriptor */
	fs_descriptor_t data = {
		.type = &fugue_descriptor_type,
		.data = (void *)fugue_fd,
	};
	int fd = fs_create_descriptor(&data);

	if(fd == -1) {
		BFile_Close(fugue_fd);
		errno = ENFILE;
		goto end;
	}

	rc = fd;
end:
	free(fcpath);
	return rc;
}

int fugue_unlink(char const *path)
{
	if(gint[HWFS] != HWFS_FUGUE) {
		errno = ENOTSUP;
		return -1;
	}

	uint16_t *fcpath = utf8_to_fc_alloc(path, u"\\\\fls0\\");
	if(!fcpath) {
		errno = ENOMEM;
		return -1;
	}

	int err = BFile_Remove(fcpath);
	if(err < 0) {
		errno = bfile_error_to_errno(err);
		return -1;
	}

	return 0;
}
