#include <unistd.h>
#include <gint/fs.h>
#include <errno.h>

ssize_t pread(int fd, void *buf, size_t size, off_t offset)
{
	fs_descriptor_t const *d = fs_get_descriptor(fd);
	if(!d) {
		errno = EBADF;
		return (ssize_t)-1;
	}

	if(d->type->pread)
		return d->type->pread(d->data, buf, size, offset);

	/* No seek function: cannot seek */
	return 0;
}
