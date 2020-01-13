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
   @size  Pointer to file size if [type = BFile_File], ignored otherwise
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

   @fd  File descriptor
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

/* BFile_FindFirst(): Search a directory for file with matching name.
   Doesn't work on Main memory.
   Only four search handle can be opened, you need to close them to be able to reuse them.
   Search is NOT case sensitive and * can be used to match any string.

   @search      FONTCHARACTER file path to match
   @shandle     Search handle to pass to BFile_FindNext or BFile_FindClose
   @founfile    FONTCHARACTER found file path
   @fileinfo    Structure containing a lot of information on the found file
   Return 0 on success, -1 if no file found, or negative value on error. */
typedef struct tag_FILE_INFO
{
    unsigned short id;       //file index
    unsigned short type;     //file type
    unsigned long  fsize;    //file size
    unsigned long  dsize;    //date size (file - header)
    unsigned int   property; //if 0 file complete
    unsigned long  adress;   //top adress of data (direct access is unadvised)
} FILE_INFO;
int BFile_FindFirst(uint16_t const *search, int *shandle, uint16_t *foundfile, FILE_INFO *fileinfo);

/* BFile_FindNext(): Continue a search a directory for file with matching name.
   Only four search handle can be opened, u need to close them to be able to reuse them.

   @shandle     Search handle from BFile_FindFirst
   @founfile    FONTCHARACTER found file path
   @fileinfo    Structure containing a lot of information on the found file
   Return 0 on success, -1 if no file found, or negative value on error. */
int BFile_FindNext(int shandle, uint16_t *foundfile, FILE_INFO *fileinfo);

/* BFile_FindClose(): Close the specified search handle
   Only four search handle can be opened, u need to close them to be able to reuse them.

   @shandle     Search handle from BFile_FindFirst
   Return 0 on success or negative value on error. */
int BFile_FindClose(int shandle);

#endif /* GINT_BFILE */
