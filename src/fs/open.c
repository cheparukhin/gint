#include <fcntl.h>
#include "util.h"

int open(char const *path, int flags, ...)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = utf8_to_fc_alloc(path, u"\\\\fls0\\");
	int fd, err, rc = -1;

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
	fd = BFile_EntryNotFound;
	if(excl || !trunc)
		fd = BFile_Open(fcpath, bfile_mode);

	/* If the file exists and O_EXCL was requested, fail. */
	if(fd >= 0 && excl) {
		BFile_Close(fd);
		errno = EEXIST;
		return -1;
	}

	/* If O_TRUNC is requested and either the file exists or O_CREAT is
	   set, temporarily remove it. */
	if((flags & O_TRUNC) && (fd >= 0 || (flags & O_CREAT))) {
		if(fd >= 0) BFile_Close(fd);
		BFile_Remove(fcpath);
		fd = BFile_EntryNotFound;
	}

	/* If the file does not exist and O_CREAT is set, create it */
	if((flags & O_CREAT) && ((flags & O_TRUNC) || fd < 0)) {
		int size = 0;
		err = BFile_Create(fcpath, BFile_File, &size);
		if(err < 0) {
			errno = bfile_error_to_errno(err);
			goto end;
		}
		fd = BFile_Open(fcpath, bfile_mode);
	}

	if(fd < 0) {
		errno = bfile_error_to_errno(fd);
		goto end;
	}

	/* If O_APPEND is set, move to the end of the file */
	if((flags & O_APPEND))
		BFile_Seek(fd, BFile_Size(fd));

	/* Return the now-open file descriptor */
	rc = fd;
end:
	free(fcpath);
	return rc;
}
