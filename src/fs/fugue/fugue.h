#ifndef FS_FUGUE_FUGUE_H
#define FS_FUGUE_FUGUE_H

#include <fcntl.h>
#include <unistd.h>

/* Specific implementation of open() */
int fugue_open(char const *path, int flags, mode_t mode);

/* Specific implementation of unlink() */
int fugue_unlink(char const *path);

#endif /* FS_FUGUE_FUGUE_H */
