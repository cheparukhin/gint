#ifndef FS_UTIL_H
#define FS_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Translate common BFile error codes to errno values. */
extern int bfile_error_to_errno(int bfile_error_code);

/* TODO: These functions do not actually translate special characters between
   encodings, they simply strip them. */

/* Convert UTF-8 to FONTCHARACTER; outputs fc_len characters with padding. If
   fc[fc_len-1] is not 0 after the call, then fc is too short. */
extern void utf8_to_fc(uint16_t *fc, char const *utf8, size_t fc_len);

/* Same in the other direction. */
extern void fc_to_utf8(char *utf8, uint16_t const *fc, size_t utf8_len);

/* Same as utf8_to_fc() but allocates a string with malloc(). */
extern uint16_t *utf8_to_fc_alloc(char const *utf8, uint16_t *prefix);

/* Same as fc_to_utf8() but allocates a string with malloc(). */
extern char *fc_to_utf8_alloc(uint16_t const *fc);

#ifdef __cplusplus
}
#endif

#endif /* FS_UTIL_H */
