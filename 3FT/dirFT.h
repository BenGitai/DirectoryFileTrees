/*--------------------------------------------------------------------*/
/* dirFT.h                                                            */
/* Author: Benjamin Gitai & Jeremy Arking                             */
/*--------------------------------------------------------------------*/

#ifndef DIR_INCLUDED
#define DIR_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "path.h"


/* A Node_T is a node in a Directory Tree */
typedef struct dir *Dir_T;

/*
  Creates a new node in the Directory Tree, with path oPPath and
  parent oDParent. Returns an int SUCCESS status and sets *poNResult
  to be the new node if successful. Otherwise, sets *poNResult to NULL
  and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oDParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oDParent's path is not oPPath's direct parent
                 or oDParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oDParent already has a child with this path
*/
int Dir_new(Path_T oPPath, Dir_T oDParent, Dir_T *poDResult);

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oDDir, i.e., deletes this node and all its descendents. Returns the
  number of nodes deleted.
*/
size_t Dir_free(Dir_T oDDir);

/* Returns the path object representing oDDir's absolute path. */
Path_T Dir_getPath(Dir_T oDDir);

/*
  Returns TRUE if oDParent has a directory child with path oPPath. Returns
  FALSE if it does not.

  If oDParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oDParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean Dir_hasDirChild(Dir_T oDParent, Path_T oPPath,
                         size_t *pulChildID);

/*
  Returns TRUE if oDParent has a file child with path oPPath. Returns
  FALSE if it does not.

  If oDParent has such a child, stores in *pulChildID the child's
  identifier (as used in Node_getChild). If oDParent does not have
  such a child, stores in *pulChildID the identifier that such a
  child _would_ have if inserted.
*/
boolean Dir_hasFileChild(Dir_T oDParent, Path_T oPPath,
                         size_t *pulChildID);

/* Returns the number of directory children that oDParent has. */
size_t Dir_getNumDirChildren(Dir_T oDParent);

/* Returns the number of file children that oDParent has. */
size_t Dir_getNumFileChildren(Dir_T oDParent);

/*
  Returns an int SUCCESS status and sets *poNResult to be the directory child
  node of oDParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oDParent
*/
int Dir_getDirChild(Dir_T oDParent, size_t ulChildID,
                  Dir_T *poNResult);

/*
  Returns an int SUCCESS status and sets *poNResult to be the file child
  node of oDParent with identifier ulChildID, if one exists.
  Otherwise, sets *poNResult to NULL and returns status:
  * NO_SUCH_PATH if ulChildID is not a valid child for oDParent
*/
int Dir_getFileChild(Dir_T oDParent, size_t ulChildID,
                  Dir_T *poNResult);

/*
  Returns a the parent node of oDDir.
  Returns NULL if oDDir is the root and thus has no parent.
*/
Dir_T Dir_getParent(Dir_T oDDir);

/*
  Compares oDFirst and oDSecond lexicographically based on their paths.
  Returns <0, 0, or >0 if oDFirst is "less than", "equal to", or
  "greater than" oDSecond, respectively.
*/
int Dir_compare(Dir_T oDFirst, Dir_T oDSecond);

/*
  Returns a string representation for oDDir, or NULL if
  there is an allocation error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *Dir_toString(Dir_T oDDir);

#endif
