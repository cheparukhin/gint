#include <gint/fs.h>
#include <gint/bfile.h>
#include <fcntl.h>
#include <errno.h>
#include "util.h"
#include "fugue.h"

/* TODO: fugue_open(): Handle trailing '/' and filesystem root */

int fugue_open(char const *path, int flags, GUNUSED mode_t mode)
{
	ENOTSUP_IF_NOT_FUGUE(-1);

	uint16_t *fcpath = utf8_to_fc_alloc(u"\\\\fls0\\", path, NULL);
	int fugue_fd, err, rc=-1, type, fd=-1;

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

	/* Exclusive open means no sense unless creation is also requested */
	bool excl = (flags & O_EXCL) && (flags & O_CREAT);
	/* Truncation requires the file to be removed/recreated */
	bool trunc = (flags & O_TRUNC) && (flags & O_CREAT);

	/* Stat the entry */
	bool exists = (BFile_Ext_Stat(fcpath, &type, NULL) == 0);
	if(!exists) type = -1;

	/* If the entry exists and O_EXCL was requested, fail. */
	if(exists && excl) {
		errno = EEXIST;
		return -1;
	}

	/* If the entry is not a directory but O_DIRECTORY is set, fail. If the
	   directory doesn't exist, we fail regardless of O_CREAT. */
	if((flags & O_DIRECTORY) && type != BFile_Type_Directory) {
		errno = (exists ? ENOTDIR : ENOENT);
		return -1;
	}

	/* If the entry is a directory, open it as such */
	if(type == BFile_Type_Directory) {
		void *dp = fugue_dir_explore(path);
		fs_descriptor_t data = {
			.type = &fugue_dir_descriptor_type,
			.data = dp,
		};
		fd = fs_create_descriptor(&data);
		rc = fd;
		goto end;
	}

	/* Try and open the file normally, unless O_TRUNC is specified without
	   O_EXCL, in which case we simply delete and recreate the file. */
	if(trunc)
		fugue_fd = BFile_EntryNotFound;
	else
		fugue_fd = BFile_Open(fcpath, bfile_mode);

	/* If O_TRUNC is requested and either the file exists or we can create
	   it, remove it. (If fugue_fd < 0 an opening error might still have
	   occurred so we delete it just in case.) */
	if((flags & O_TRUNC) && (fugue_fd >= 0 || (flags & O_CREAT))) {
		if(fugue_fd >= 0)
			BFile_Close(fugue_fd);
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
		rc = fugue_fd;
		goto end;
	}

	/* If O_APPEND is set, move to the end of the file */
	// TODO: O_APPEND should move the cursor before *each* write
	if((flags & O_APPEND))
		BFile_Seek(fugue_fd, BFile_Size(fugue_fd));

	/* Return the now-open file descriptor */
	fs_descriptor_t data = {
		.type = &fugue_descriptor_type,
		.data = (void *)fugue_fd,
	};
	fd = fs_create_descriptor(&data);

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
