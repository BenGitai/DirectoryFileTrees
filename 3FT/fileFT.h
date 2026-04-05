/*--------------------------------------------------------------------*/
/* file.h                                                             */
/* Author: Jeremy Arking and Ben Gitai                                */
/*--------------------------------------------------------------------*/

#ifndef FILE_INCLUDED
#define FILE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"
#include "dirFT.h"

/*
  Creates a new file in the File Tree, with path oPPath and
  parent oFParent. Returns an int SUCCESS status and sets *oFFile
  to be the new File if successful. Otherwise, sets *oFFile to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CoFFLICTING_PATH if oFParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oFParent's path is not oPPath's direct parent
                 or oFParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oFParent already has a child with this path
*/
int File_new(Path_T oPPath, Dir_T oFParent, void *contents, size_t ulLength, File_T *oFFile);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oFFile, i.e., deletes this File and all its descendents. Returns the
  number of Files deleted.
*/
size_t File_free(File_T oFFile);

/* Returns the path object representing oFFile's absolute path. */
Path_T File_getPath(File_T oFFile);

/*
  Returns a the parent File of oFFile.
  Returns NULL if oFFile is the root and thus has no parent.
*/
Dir_T File_getParent(File_T oFFile);

/* return the contents of oFFile */
void *File_getContents(File_T oFFile);

/* replace the contents of oFFile with pvNewContents and length ulLength */
void *File_replaceContents(File_T oFFile, void *pvNewContents, size_t ulLength);

/* get the size in bytes of the contents of the file */
int File_getContentSize(File_T oFFile);

/*
  Compares oFFirst and oFSecoFd lexicographically based oF their paths.
  Returns <0, 0, or >0 if oFFirst is "less than", "equal to", or
  "greater than" oFSecoFd, respectively.
*/
int File_compare(File_T oFFirst, File_T oFSecond);

/*
  Returns a string representatioF for oFFile, or NULL if
  there is an allocatioF error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *File_toString(File_T oFFile);

#endif
