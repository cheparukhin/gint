//---
//	gint:bfile - BFile interface
//
//	The system's file system is a nightmare and the risk of corrupting it
//	or the flash by driving it manually is too great to risk. This module
//	interfaces with the BFile syscalls for file management.
//---

#ifndef GINT_BFILE
#define GINT_BFILE

#include <stdint.h>

/* BFile_Remove(): Remove a file
   Also works if the file does not exist.

   @file  FONTCHARACTER file path
   Returns a BFile error code. */
int BFile_Remove(uint16_t const *file);

/* BFile_Create(): Create a new entry
   The file or directory must not exist. For a file the size pointer must point
   to the desired file size (which is fixed), for a folder it is ignored.

   @file  FONTCHARACTER file path
   @type  Entry type
   @size  Pointer to file size if [type = BFile_File], use NULL otherwise
   Returns a BFile error code. */
enum BFile_EntryType
{
	BFile_File	= 1,
	BFile_Folder	= 5,
};
int BFile_Create(uint16_t const *file, enum BFile_EntryType type, int *size);

/* BFile_Open(): Open an existing file
   The file must exist.

   @file  FONTCHARACTER file path
   @mode  Desired access mode
   Returns a file descriptor on success, or a negative BFile error code. */
enum BFile_OpenMode
{
	BFile_ReadOnly	= 0x01,
	BFile_WriteOnly	= 0x02,
	BFile_ReadWrite	= BFile_ReadOnly | BFile_WriteOnly,
};
int BFile_Open(uint16_t const *file, enum BFile_OpenMode mode);

/* BFile_Close(): Close a file descriptor
   @fd  Open file descriptor
   Returns a BFile error code. */
int BFile_Close(int fd);

/* BFile_Size(): Retrieve size of an open file
   @fd  Open file descriptor
   Returns the file size in bytes, or a negative BFile error code*/
int BFile_Size(int fd);

/* BFile_Write(): Write data to an open file
   Second and third argument specify the data and length to write.

   WARNING: The file systems has shown to become inconsistent if an odd number
            of bytes is written with BFile_Write(). Keep it even!

   @fd         File descriptor open for writing
   @data       Data to write
   @even_size  Size to write (must be even, yes)
   Returns a BFile error code. */
int BFile_Write(int fd, void const *data, int even_size);

/* BFile_Read(): Read data from an open file
   The second and third argument specify where to store and how much to read.
   The location from where the data is read depends on [whence]:

   * If [whence >= 0], it is taken as an absolute location within the file;
   * If [whence == -1], BFile_Read() reads from the current position.

   @fd      File descriptor open for reading
   @data    Buffer of at least [size] bytes to store data to
   @size    Number of bytes to read
   @whence  Starting position of the data to read in the file
   Returns a BFile error code. */
int BFile_Read(int handle, void *data, int size, int whence);

/* BFile_FindFirst(): Search a directory for file with matching name
   Doesn't work on Main memory. Only four search handle can be opened, you need
   to close them to be able to reuse them. Search is NOT case sensitive and *
   can be used as a wildcard.

   @search    FONTCHARACTER file path to match
   @shandle   Search handle to pass to BFile_FindNext or BFile_FindClose
   @founfile  FONTCHARACTER found file path
   @fileinfo  Structure containing a lot of information on the found file
   Returns 0 on success, -1 if no file found, or negative value on error. */
struct BFile_FileInfo
{
	uint16_t index;
	uint16_t type;
	uint32_t file_size;
	/* Data size (file size minus the header) */
	uint32_t data_size;
	/* Is 0 if the file is complete */
	uint32_t property;
	/* Address of first fragment (do not use directly) */
	void *address;
};
enum BFile_FileType
{
	BFile_Type_Directory  = 0x0000,
	BFile_Type_File       = 0x0001,
	BFile_Type_Addin      = 0x0002,
	BFile_Type_Eact       = 0x0003,
	BFile_Type_Language   = 0x0004,
	BFile_Type_Bitmap     = 0x0005,
	BFile_Type_MainMem    = 0x0006,
	BFile_Type_Temp       = 0x0007,
	BFile_Type_Dot        = 0x0008,
	BFile_Type_DotDot     = 0x0009,
	BFile_Type_Volume     = 0x000a,
	BFile_Type_Archived   = 0x0041,
};
int BFile_FindFirst(uint16_t const *search, int *shandle, uint16_t *foundfile,
	struct BFile_FileInfo *fileinfo);

/* BFile_FindNext(): Continue a search a directory for file with matching name

   @shandle   Search handle from BFile_FindFirst
   @founfile  FONTCHARACTER found file path
   @fileinfo  Structure containing a lot of information on the found file
   Returns 0 on success, -1 if end is reached, or negative value on error. */
int BFile_FindNext(int shandle, uint16_t *foundfile,
	struct BFile_FileInfo *fileinfo);

/* BFile_FindClose(): Close the specified search handle
   @shandle  Search handle from BFile_FindFirst
   Return 0 on success or negative value on error. */
int BFile_FindClose(int shandle);

#endif /* GINT_BFILE */
