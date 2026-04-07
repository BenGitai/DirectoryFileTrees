/*--------------------------------------------------------------------*/
/* file.c                                                             */
/* Author: Jeremy Arking and Ben Gitai                                */
/*--------------------------------------------------------------------*/
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "dirFT.h"
#include "fileFT.h"
#include "path.h"

struct file {
    /* the Path object corresponding to the file */
    Path_T oPPath;
    /* this file's parent dir */
    Dir_T oDParent;
    /* file contents */
    void *contents;
    /* file size */
    size_t ulLength;
};

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int File_compareString(const File_T oFFirst,
                                 const char *pcSecond) {
   assert(oFFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oFFirst->oPPath, pcSecond);
}

int File_new(Path_T oPPath, Dir_T oDParent, void *contents, size_t ulLength, File_T *oFFile) {
  File_T result;
  Path_T oPNewPath;
  int iStatus;
  assert(oPPath != NULL);
  assert(oDParent != NULL);
  
  result = malloc(sizeof(struct file));
  if (result == NULL) {
    return MEMORY_ERROR;
  }

  /* set the new node's path */
  iStatus = Path_dup(oPPath, &oPNewPath);
  if(iStatus != SUCCESS) {
      free(result);
      return iStatus;
   }
  result->oPPath = oPNewPath;
  result->oDParent = oDParent;
  result->contents = contents;
  result->ulLength = ulLength;
  *oFFile = result;
  return SUCCESS;
}

/*
  Destroys and frees all memory allocated for the subtree rooted at
  oFFile, i.e., deletes this File and all its descendents. Returns the
  number of Files deleted.
*/
size_t File_free(File_T oFFile) {
  assert(oFFile != NULL);
  Path_free(oFFile->oPPath);
  free(oFFile);
  return 1;
}

/* Returns the path object representing oFFile's absolute path. */
Path_T File_getPath(File_T oFFile) {
  assert(oFFile != NULL);

  return oFFile->oPPath;
}


/*
  Returns a the parent File of oFFile.
  Returns NULL if oFFile is the root and thus has no parent.
*/
Dir_T File_getParent(File_T oFFile) {
  assert(oFFile != NULL);

  return oFFile->oDParent;
}

/* return the contents of oFFile */
void *File_getContents(File_T oFFile) {
    assert(oFFile != NULL);
    return oFFile->contents;
}

/* replace the contents of oFFile with pvNewContents and length ulLength */
void *File_replaceContents(File_T oFFile, void *pvNewContents, size_t ulLength) {
    void *result;
    assert(oFFile != NULL);
    result = oFFile->contents;
    oFFile->contents = pvNewContents;
    oFFile->ulLength = ulLength;
    return result;
}

/* get the size in bytes of the contents of the file */
int File_getContentSize(File_T oFFile) {
    assert(oFFile != NULL);
    return oFFile->ulLength;
}

/*
  Compares oFFirst and oFSecoFd lexicographically based oF their paths.
  Returns <0, 0, or >0 if oFFirst is "less than", "equal to", or
  "greater than" oFSecoFd, respectively.
*/
int File_compare(File_T oFFirst, Path_T oFSecond) {
  assert(oFFirst != NULL);
  assert(oFSecond != NULL);

  return Path_comparePath(oFFirst->oPPath, oFSecond);
}

/*
  Returns a string representatioF for oFFile, or NULL if
  there is an allocatioF error.

  Allocates memory for the returned string, which is then owned by
  the caller!
*/
char *File_toString(File_T oFFile) {
  char *copyPath;

  assert(oFFile != NULL);

  copyPath = malloc(Path_getStrLength(File_getPath(oFFile)) + 1);
  if (copyPath == NULL)
    return NULL;
  else
    return strcpy(copyPath, Path_getPathname(File_getPath(oFFile)));
}
