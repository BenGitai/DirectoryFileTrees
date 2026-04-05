/*--------------------------------------------------------------------*/
/* dirFT.c                                                            */
/* Author: Benjamin Gitai & Jeremy Arking                             */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dynarray.h"
#include "dirFT.h"
#include "fileFT.h"
#include "path.h"

/* A directory node in a FT */
struct dir {
   /* the object corresponding to the directory's absolute path */
   Path_T oPPath;
   /* this directory's parent */
   Dir_T oDParent;
   /* the object containing links to this directory's directory children */
   DynArray_T oDDirChildren;
   /* the object containing links to this directory's file children */
   DynArray_T oDFileChildren;
};


/*
  Links new child oNChild into oDParent's children array at index
  ulIndex. Returns SUCCESS if the new child was added successfully,
  or  MEMORY_ERROR if allocation fails adding oNChild to the array.
*/
static int Dir_addDirChild(Dir_T oDParent, Dir_T oDChild,
                         size_t ulIndex) {
   assert(oDParent != NULL);
   assert(oDChild != NULL);

   if(DynArray_addAt(oDParent->oDDirChildren, ulIndex, oDChild))
      return SUCCESS;
   else
      return MEMORY_ERROR;
}

int Dir_addFileChild(Dir_T oDParent, File_T oFChild,
                         size_t ulIndex) {
   assert(oDParent != NULL);
   assert(oFChild != NULL);

   if(DynArray_addAt(oDParent->oDFileChildren, ulIndex, oFChild))
      return SUCCESS;
   else
      return MEMORY_ERROR;
}

/*
  Compares the string representation of oNfirst with a string
  pcSecond representing a node's path.
  Returns <0, 0, or >0 if oNFirst is "less than", "equal to", or
  "greater than" pcSecond, respectively.
*/
static int Dir_compareString(const Dir_T oDFirst,
                                 const char *pcSecond) {
   assert(oDFirst != NULL);
   assert(pcSecond != NULL);

   return Path_compareString(oDFirst->oPPath, pcSecond);
}


/*
  Creates a new node with path oPPath and parent oDParent.  Returns an
  int SUCCESS status and sets *poNResult to be the new node if
  successful. Otherwise, sets *poNResult to NULL and returns status:
  * MEMORY_ERROR if memory could not be allocated to complete request
  * CONFLICTING_PATH if oDParent's path is not an ancestor of oPPath
  * NO_SUCH_PATH if oPPath is of depth 0
                 or oDParent's path is not oPPath's direct parent
                 or oDParent is NULL but oPPath is not of depth 1
  * ALREADY_IN_TREE if oDParent already has a child with this path
*/
int Dir_new(Path_T oPPath, Dir_T oDParent, Dir_T *poDResult) {
   struct dir *psNew;
   Path_T oPParentPath = NULL;
   Path_T oPNewPath = NULL;
   size_t ulParentDepth;
   size_t ulIndex;
   int iStatus;

   assert(oPPath != NULL);
   /* assert(oDParent == NULL || CheckerDT_Node_isValid(oDParent)); */

   /* allocate space for a new node */
   psNew = malloc(sizeof(struct dir));
   if(psNew == NULL) {
      *poDResult = NULL;
      return MEMORY_ERROR;
   }

   /* set the new node's path */
   iStatus = Path_dup(oPPath, &oPNewPath);
   if(iStatus != SUCCESS) {
      free(psNew);
      *poDResult = NULL;
      return iStatus;
   }
   psNew->oPPath = oPNewPath;

   /* validate and set the new node's parent */
   if(oDParent != NULL) {
      size_t ulSharedDepth;

      oPParentPath = oDParent->oPPath;
      ulParentDepth = Path_getDepth(oPParentPath);
      ulSharedDepth = Path_getSharedPrefixDepth(psNew->oPPath,
                                                oPParentPath);
      /* parent must be an ancestor of child */
      if(ulSharedDepth < ulParentDepth) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDResult = NULL;
         return CONFLICTING_PATH;
      }

      /* parent must be exactly one level up from child */
      if(Path_getDepth(psNew->oPPath) != ulParentDepth + 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDResult = NULL;
         return NO_SUCH_PATH;
      }

      /* parent must not already have child with this path */
      if(Dir_hasDirChild(oDParent, oPPath, &ulIndex)) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDResult = NULL;
         return ALREADY_IN_TREE;
      }
   }
   else {
      /* new node must be root */
      /* can only create one "level" at a time */
      if(Path_getDepth(psNew->oPPath) != 1) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDResult = NULL;
         return NO_SUCH_PATH;
      }
   }
   psNew->oDParent = oDParent;

   /* initialize the new node */
   psNew->oDDirChildren = DynArray_new(0);
   if(psNew->oDDirChildren == NULL) {
      Path_free(psNew->oPPath);
      free(psNew);
      *poDResult = NULL;
      return MEMORY_ERROR;
   }

   /* Link into parent's children list */
   if(oDParent != NULL) {
      iStatus = Dir_addDirChild(oDParent, psNew, ulIndex);
      if(iStatus != SUCCESS) {
         Path_free(psNew->oPPath);
         free(psNew);
         *poDResult = NULL;
         return iStatus;
      }
   }

   *poDResult = psNew;

   /* assert(oDParent == NULL || CheckerDT_Node_isValid(oDParent)); */
   /* assert(CheckerDT_Node_isValid(*poDResult)); */

   return SUCCESS;
}

size_t Dir_free(Dir_T oDDir) {
   size_t ulIndex;
   size_t ulCount = 0;

   assert(oDDir != NULL);
   /* assert(CheckerDT_Node_isValid(oDDir)); */

   /* remove from parent's list */
   if(oDDir->oDParent != NULL) {
      if(DynArray_bsearch(
            oDDir->oDParent->oDDirChildren,
            oDDir, &ulIndex,
            (int (*)(const void *, const void *)) Dir_compare)
        )
         (void) DynArray_removeAt(oDDir->oDParent->oDDirChildren,
                                  ulIndex);
   }

   /* remove all file children */
   while(DynArray_getLength(oDDir->oDFileChildren) != 0) {
      ulCount += File_free(DynArray_get(oDDir->oDFileChildren, 0));
   }
   DynArray_free(oDDir->oDFileChildren);
   /* recursively remove directory children */
   while(DynArray_getLength(oDDir->oDDirChildren) != 0) {
      ulCount += Dir_free(DynArray_get(oDDir->oDDirChildren, 0));
   }
   DynArray_free(oDDir->oDDirChildren);

   /* remove path */
   Path_free(oDDir->oPPath);

   /* finally, free the struct node */
   free(oDDir);
   ulCount++;
   return ulCount;
}

size_t Dir_freeFile(Dir_T oDDir, size_t ulIdx) {
   File_T oFFile;
   oFFile = DynArray_removeAt(oDDir->oDFileChildren, ulIdx);
   File_free(oFFile);
   return 1;
}

Path_T Dir_getPath(Dir_T oDDir) {
   assert(oDDir != NULL);

   return oDDir->oPPath;
}

boolean Dir_hasDirChild(Dir_T oDParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oDParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);

   /* *pulChildID is the index into oDParent->oDDirChildren */
   return DynArray_bsearch(oDParent->oDDirChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) Dir_compareString);
}

boolean Dir_hasFileChild(Dir_T oDParent, Path_T oPPath,
                         size_t *pulChildID) {
   assert(oDParent != NULL);
   assert(oPPath != NULL);
   assert(pulChildID != NULL);

   /* *pulChildID is the index into oDParent->oDFileChildren */
   return DynArray_bsearch(oDParent->oDFileChildren,
            (char*) Path_getPathname(oPPath), pulChildID,
            (int (*)(const void*,const void*)) File_compare);
}

size_t Dir_getNumDirChildren(Dir_T oDParent) {
   assert(oDParent != NULL);

   return DynArray_getLength(oDParent->oDDirChildren);
}

size_t Dir_getNumFileChildren(Dir_T oDParent) {
   assert(oDParent != NULL);

   return DynArray_getLength(oDParent->oDFileChildren);
}

int  Dir_getDirChild(Dir_T oDParent, size_t ulChildID,
                   Dir_T *poNResult) {

   assert(oDParent != NULL);
   assert(poNResult != NULL);

   /* ulChildID is the index into oDParent->oDDirChildren */
   if(ulChildID >= Dir_getNumDirChildren(oDParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oDParent->oDDirChildren, ulChildID);
      return SUCCESS;
   }
}

int  Dir_getFileChild(Dir_T oDParent, size_t ulChildID,
                   File_T *poNResult) {

   assert(oDParent != NULL);
   assert(poNResult != NULL);

   /* ulChildID is the index into oDParent->oDFileChildren */
   if(ulChildID >= Dir_getNumFileChildren(oDParent)) {
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   else {
      *poNResult = DynArray_get(oDParent->oDFileChildren, ulChildID);
      return SUCCESS;
   }
}

Dir_T Dir_getParent(Dir_T oDDir) {
   assert(oDDir != NULL);

   return oDDir->oDParent;
}

int Dir_compare(Dir_T oDFirst, Dir_T oDSecond) {
   assert(oDFirst != NULL);
   assert(oDSecond != NULL);

   return Path_comparePath(oDFirst->oPPath, oDSecond->oPPath);
}

char *Dir_toString(Dir_T oDDir) {
   char *copyPath;

   assert(oDDir != NULL);

   copyPath = malloc(Path_getStrLength(Dir_getPath(oDDir))+1);
   if(copyPath == NULL)
      return NULL;
   else
      return strcpy(copyPath, Path_getPathname(Dir_getPath(oDDir)));
}
