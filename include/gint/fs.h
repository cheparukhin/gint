//---
// gint:fs - Filesystem abstraction
//---

#ifndef GINT_FS
#define GINT_FS

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <stddef.h>

/* Maximum number of file descriptors */
#define FS_FD_MAX 32

/* fs_descriptor_type_t: Overloaded file descriptor functions

   This structure provides the base functions for every type of file
   descriptor. The prototypes are standard, except that the first argument
   (int fd) is replaced by (void *data) which points to the custom data
   allocator for the descriptor.

   pread() is normally implemented in terms of lseek() and read(), however on
   Fugue file descriptors on the G-III series there is no way to determine the
   current position so this method won't work, and pread() is instead provided
   by a non-standard parameter of BFile_Read(). The overloaded entry supports
   this alternative. */
typedef struct {
    /* See <unistd.h> for a description of these functions */
    ssize_t (*read)(void *data, void *buf, size_t size);
    ssize_t (*pread)(void *data, void *buf, size_t size, off_t offset);
    ssize_t (*write)(void *data, void const *buf, size_t size);
    off_t (*lseek)(void *data, off_t offset, int whence);
    int (*close)(void *data);

} fs_descriptor_type_t;

/* fs_descriptor_t: File descriptor information
   This internal type describes the entries of the descriptor table. */
typedef struct {
    /* Interface functions */
    fs_descriptor_type_t const *type;
    /* Custom data (can also be an integer cast to (void *)) */
    void *data;

} fs_descriptor_t;

/* fs_get_descriptor(): Get a file descriptor's data

   This function is used internally in order to implement read(), write(), and
   other standard functions functions. It could be useful for other APIs that
   want to present their resources as file descriptors but still implement
   extra non-standard functions; this allows them to use the file descriptor as
   input and still access the custom data.

   Returns NULL if there is no file descriptor with this number, in which case
   the caller probably needs to set errno = EBADF. */
fs_descriptor_t const *fs_get_descriptor(int fd);

/* fs_create_descriptor(): Create a new file descriptor

   This function is used in open() and its variants to allocate new file
   descriptors. The descriptor's data is created with a copy of the provided
   structure, which must include a non-NULL type attribute.

   This function always returns the smallest file descriptor that is unused by
   the application and is not 0, 1 or 2; unless it runs out of file
   descriptors, in which case it returns -1 and the caller might want to set
   errno = ENFILE. */
int fs_create_descriptor(fs_descriptor_t const *data);

/* fs_free_descriptor(): Clsoe a file descriptor's slot

   This function frees the specified file descriptor. It is automatically
   called by close() after the descriptor type's close() function, so there is
   normally no need to call this function directly. */
void fs_free_descriptor(int fd);

/* open_generic(): Open a file descriptor using custom file functions

   Opens a new file descriptor of the specified type with the provided user
   data. If reuse_fd < 0, a new file descriptor is allocated, otherwise the
   exact file descriptor reuse_fd is used. (This is useful to reopen standard
   streams.) In this case, the only possible return values are -1 and reuse_fd
   itself. */
int open_generic(fs_descriptor_type_t *type, void *data, int reuse_fd);

#ifdef __cplusplus
}
#endif

#endif /* GINT_FS */
