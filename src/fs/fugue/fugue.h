#ifndef FS_FUGUE_FUGUE_H
#define FS_FUGUE_FUGUE_H

#include <gint/fs.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

/* File descriptor type */
extern const fs_descriptor_type_t fugue_descriptor_type;
/* Directory descriptor type */
extern const fs_descriptor_type_t fugue_dir_descriptor_type;

/* Specific implementations of some standard functions */

int fugue_open(char const *path, int flags, mode_t mode);

int fugue_unlink(char const *path);

int fugue_mkdir(char const *path, mode_t mode);

int fugue_rmdir(char const *path);

/* Other functions */

void *fugue_dir_explore(char const *path);

#endif /* FS_FUGUE_FUGUE_H */
