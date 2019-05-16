#ifndef __EFILE_H__
#define __EFILE_H__ 1

#include "Master.h"

/**
 * @file      eFile.h
 * @brief     high-level file system
 * @details   This file system sits on top of eDisk.
 * @version   V1.0
 * @author    Valvano
 * @copyright Copyright 2017 by Jonathan W. Valvano, valvano@mail.utexas.edu,
 * @warning   AS-IS
 * @note      For more information see  http://users.ece.utexas.edu/~valvano/
 * @date      March 9, 2017

 ******************************************************************************/


/**
 * @details This function must be called first, before calling any of the other eFile functions
 * @param  none
 * @return 0 if successful and 1 on failure (already initialized)
 * @brief  Activate the file system, without formating
 */
int eFile_Init(void);

/**
 * @details Deactivate the file system. One can reactive the file system with eFile_Init.
 * @param  none
 * @return 0 if successful and 1 on failure (e.g., trouble writing to flash)
 * @brief  Close the disk
 */
int eFile_Close(void);

/**
 * @details Erase all files, create blank directory, initialize free space manager
 * @param  none
 * @return 0 if successful and 1 on failure (e.g., trouble writing to flash)
 * @brief  Format the disk
 */
int eFile_Format(void);

/**
 * @details Create a new, empty file with one allocated block
 * @param  name file name is an ASCII string up to seven characters
 * @return 0 if successful and 1 on failure (e.g., already exists)
 * @brief  Create a new file
 */
int eFile_Create(char name[]);

/**
 * @details Delete the file with this name, recover blocks so they can be used by another file
 * @param  name file name is an ASCII string up to seven characters
 * @return 0 if successful and 1 on failure (e.g., file doesn't exist)
 * @brief  delete this file
 */
int eFile_Delete(char name[]); 

/**
 * @details Open the file for writing, read into RAM last block
 * @param  name file name is an ASCII string up to seven characters
 * @return File Handle, null if error
 * @brief  Open an existing file for writing
 */
FILE* eFile_WOpen(char name[]);

/**
 * @details Save one byte at end of the open file
 * @param  data byte to be saved on the disk
 * @return 0 if successful and 1 on failure (e.g., trouble writing to flash)
 * @brief  Format the disk
 */
int eFile_Write(char data);

/**
 * @details Close the file, leave disk in a state power can be removed.
 * This function will flush all RAM buffers to the disk.
 * @param  none
 * @return 0 if successful and 1 on failure (e.g., trouble writing to flash)
 * @brief  Close the file that was being written
 */
int eFile_WClose(void);

/**
 * @details Open the file for reading, read first block into RAM
 * @param  name file name is an ASCII string up to seven characters
 * @return File Handle, null if error
 * @brief  Open an existing file for reading
 */
FILE* eFile_ROpen(char name[]);

/**
 * @details Read one byte from disk into RAM
 * @param  pt call by reference pointer to place to save data
 * @return 0 if successful and 1 on failure (e.g., trouble reading from flash)
 * @brief  Retreive data from open file
 */
int eFile_ReadNext(char *pt);

/**
 * @details Close the file, leave disk in a state power can be removed.
 * @param  none
 * @return 0 if successful and 1 on failure (e.g., wasn't open)
 * @brief  Close the file that was being read
 */
int eFile_RClose(void);

/**
 * @details Display the directory with filenames and sizes
 * @param  Output Stream to print to
 * @return 0 if successful and 1 on failure (e.g., trouble reading from flash)
 * @brief  Show directory
 */
int eFile_Directory(FILE *target);

/**
 * @details Display the contents of given File
 * @param  Output Stream to print to, File to display
 * @return 0 if successful and 1 on failure (e.g., file doesn't exist)
 * @brief  Display this file
 */
int eFile_DisplayFile(FILE *target, char file_name[]);

/**
 * @details gets the file handle for currently open read file
 * @param  none
 * @return file handle, FILE* null if none
 * @brief  get open read file handle
 */
FILE* eFile_getReadFile(void);

/**
 * @details gets the file handle for currently open write file
 * @param  none
 * @return file handle, FILE* null if none
 * @brief  get open write file handle
 */
FILE* eFile_getWriteFile(void);

/**
 * @details Looks up file by name and returns a file handle
 * @param  string containing file name
 * @return file handle, FILE* null if none found
 * @brief  gets file handle for given file
 */
FILE* eFile_getFilePtr(char* name);

/**
 * @details Looks up file by handle and returns a name
 * @param  file handle, buffer to put name in (8 bytes min)
 * @return if successful and 1 on failure (e.g., file doesn't exist)
 * @brief  gets file name for given file handle
 */
int eFile_getName(FILE* file, char* buff);

/**
 * @details Checks if file handle is current open for reading
 * @param  file handle
 * @return 0 for succes, 1 for failure
 * @brief  verifies file is open to read
 */
int eFile_isReadOpen(FILE* f);

/**
 * @details Checks if file handle is current open for writing
 * @param  file handle
 * @return 0 for succes, 1 for failure
 * @brief  verifies file is open to write
 */
int eFile_isWriteOpen(FILE* f);

/**
 * @details Gets the size of the given file
 * @param  file handle
 * @return size in bytes of file
 * @brief  get file size
 */
unsigned long eFile_getSize_File(FILE* f);

/**
 * @details Gets the size of the given file
 * @param  file name
 * @return size in bytes of file
 * @brief  get file size
 */
unsigned long eFile_getSize_Name(char* name);

#endif // __EFILE_H__
