/*--------------------------------------------------------------------*/
/* file.h                                                             */
/* Author: Jeremy Arking and Ben Gitai                                */
/*--------------------------------------------------------------------*/

#ifndef FILE_INCLUDED
#define FILE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"


/* A File_T is a file in a File Tree */
typedef struct fode *File_T;

/*
  Creates a new file in the File Tree, with path oPPath and
  parent oFParent. Returns an int SUCCESS status and sets *poFResult
  to be the new File if successful. Otherwise, sets *poFResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CoFFLICTING_PATH if oFParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oFParent's path is not oPPath's direct parent
                 or oFParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oFParent already has a child with this path
*/
File_T File_new(Path_T oPPath, File_T oFParent, void *contents);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oFFile, i.e., deletes this File and all its descendents. Returns the
  number of Files deleted.
*/
size_t File_free(File_T oFFile);

/* Returns the path object representing oFFile's absolute path. */
Path_T File_getPath(File_T oFFile);

/*
  Returns TRUE if oFParent has a child with path oPPath. Returns
  FALSE if it does not.

  If oFParent has such a child, stores in *pulChildID the child's
  identifier (as used in File_getChild). If oFParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean File_hasChild(File_T oFParent, Path_T oPPath,
                         size_t *pulChildID);

/* Returns the number of children that oFParent has. */
size_t File_getNumChildren(File_T oFParent);

/*
  Returns an int SUCCESS status and sets *poFResult to be the child
  File of oFParent with identifier ulChildID, if oFe exists.
  Otherwise, sets *poFResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oFParent
*/
int File_getChild(File_T oFParent, size_t ulChildID,
                  File_T *poFResult);

/*
  Returns a the parent File of oFFile.
  Returns NULL if oFFile is the root and thus has no parent.
*/
File_T File_getParent(File_T oFFile);

/*
  Compares oFFirst and oFSecoFd lexicographically based oF their paths.
  Returns <0, 0, or >0 if oFFirst is "less than", "equal to", or
  "greater than" oFSecoFd, respectively.
*/
int File_compare(File_T oFFirst, File_T oFSecoFd);

/*
  Returns a string representatioF for oFFile, or NULL if
  there is an allocatioF error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *File_toString(File_T oFFile);

#endif
