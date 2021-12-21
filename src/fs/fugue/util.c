#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gint/bfile.h>

int bfile_error_to_errno(int e)
{
	/* TODO: Find BFile code for too many fds and map it to ENFILE. */
	switch(e) {
	case BFile_EntryNotFound:	return ENOENT;
	case BFile_IllegalPath:		return EINVAL;
	case BFile_DeviceFull:		return EDQUOT;
	case BFile_IllegalDevice:	return EINVAL;
	case BFile_AccessDenied:	return EACCES;
	case BFile_PermissionError:	return EACCES;
	case BFile_EntryFull:		return EDQUOT;
	case BFile_AlreadyExists:	return EEXIST;
	case BFile_ReadOnlyFile:	return EACCES;
	case BFile_EnumerateEnd:	return ENOENT;
	case BFile_IllegalSeekPos:	return EINVAL;
	case BFile_NotMountDevice:	return ENOENT;
	case BFile_DeviceNotFound:	return ENOENT;
	default:			return errno;
	}
}

/* Length of FONTCHARACTER and UTF-8 strings, counting only ASCII characters */
size_t utf8_len(char const *utf8)
{
	size_t len = 0;
	for(size_t i = 0; utf8[i] != 0; i++)
		len += (utf8[i] >= 0 && (uint8_t)utf8[i] <= 0x7f);
	return len;
}
size_t fc_len(uint16_t const *fc)
{
	size_t len = 0;
	for(size_t i = 0; fc[i] != 0 && fc[i] != 0xffff; i++)
		len += (fc[i] <= 0x7f);
	return len;
}

void utf8_to_fc(uint16_t *fc, char const *utf8, size_t fc_len)
{
	size_t j = 0;

	for(size_t i = 0; j < fc_len && utf8[i] != 0; i++) {
		if(utf8[i] == '/')
			fc[j++] = '\\';
		else if(utf8[i] >= 0 && (uint8_t)utf8[i] <= 0x7f)
			fc[j++] = utf8[i];
	}

	while(j < fc_len)
		fc[j++] = 0;
}

void fc_to_utf8(char *utf8, uint16_t const *fc, size_t utf8_len)
{
	size_t j = 0;

	for(size_t i = 0; j < utf8_len && fc[i] != 0 && fc[i] != 0xffff; i++) {
		if(fc[i] == '\\')
			utf8[j++] = '/';
		else if(fc[i] <= 0x7f)
			utf8[j++] = fc[i];
	}

	while(j < utf8_len)
		utf8[j++] = 0;
}

uint16_t *utf8_to_fc_alloc(uint16_t *prefix, char const *utf8,
	uint16_t *suffix)
{
	size_t lenp=0, lens=0;
	if(prefix) {
		while(prefix[lenp] != 0 && prefix[lenp] != 0xffff)
			lenp++;
	}
	if(suffix) {
		while(suffix[lens] != 0 && suffix[lens] != 0xffff)
			lens++;
	}

	size_t len = utf8_len(utf8);
	uint16_t *fc = malloc((lenp+len+lens+1) * sizeof *fc);

	if(fc != NULL) {
		if(prefix)
			memcpy(fc, prefix, lenp * sizeof *prefix);
		utf8_to_fc(fc + lenp, utf8, len);
		if(suffix)
			memcpy(fc + lenp + len, suffix, lens * sizeof *suffix);
		fc[lenp+len+lens] = 0;
	}
	return fc;
}

char *fc_to_utf8_alloc(uint16_t const *fc)
{
	size_t len = fc_len(fc);
	char *utf8 = malloc(len+1);
	if(utf8 != NULL)
		fc_to_utf8(utf8, fc, len+1);
	return utf8;
}
