/*--------------------------------------------------------------------*/
/* dt.c                                                               */
/* Author: Jeremy Arking                                       */
/*--------------------------------------------------------------------*/

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "path.h"
#include "ft.h"
#include "dirFT.h"
#include "fileFT.h"

/*
  A Dir Tree is a representation of a hierarchy of directories,
  represented as an AO with 3 state variables:
*/

/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Dir_T oDRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;





/* --------------------------------------------------------------------

  The FT_traversePath and FT_findDir functions modularize the common
  functionality of going as far as possible down an FT towards a path
  and returning either the node of however far was reached or the
  node if the full path was reached, respectively.
*/

/*
  Traverses the FT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poDFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poDFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
*/
static int FT_traversePath(Path_T oPPath, Dir_T *poDFurthest) {
   int iStatus;
   Path_T oPPrefix = NULL;
   Dir_T oDCurr;
   Dir_T oDChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;

   assert(oPPath != NULL);
   assert(poDFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oDRoot == NULL) {
      *poDFurthest = NULL;
      return SUCCESS;
   }

   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if(iStatus != SUCCESS) {
      *poDFurthest = NULL;
      return iStatus;
   }

   if(Path_comparePath(Dir_getPath(oDRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poDFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;

   oDCurr = oDRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poDFurthest = NULL;
         return iStatus;
      }
      if(Dir_hasDirChild(oDCurr, oPPrefix, &ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = Dir_getDirChild(oDCurr, ulChildID, &oDChild);
         if(iStatus != SUCCESS) {
            *poDFurthest = NULL;
            return iStatus;
         }
         oDCurr = oDChild;
      }
      else {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }

   Path_free(oPPrefix);
   *poDFurthest = oDCurr;
   return SUCCESS;
}

/*
  Traverses the DT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the DT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findDir(const char *pcPath, Dir_T *poDResult) {
   Path_T oPPath = NULL;
   Dir_T oDFound = NULL;
   int iStatus;

   assert(pcPath != NULL);
   assert(poDResult != NULL);

   if(!bIsInitialized) {
      *poDResult = NULL;
      return INITIALIZATION_ERROR;
   }

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poDResult = NULL;
      return iStatus;
   }

   iStatus = FT_traversePath(oPPath, &oDFound);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poDResult = NULL;
      return iStatus;
   }

   if(oDFound == NULL) {
      Path_free(oPPath);
      *poDResult = NULL;
      return NO_SUCH_PATH;
   }

   if(Path_comparePath(Dir_getPath(oDFound), oPPath) != 0) {
      Path_free(oPPath);
      *poDResult = NULL;
      return NO_SUCH_PATH;
   }

   Path_free(oPPath);
   *poDResult = oDFound;
   return SUCCESS;
}

static int FT_insertPath(Path_T oPPath, Dir_T *oDEnd) {
   int iStatus;
   Dir_T oDFirstNew = NULL;
   Dir_T oDCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewDirs = 0;

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oDCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oDCurr == NULL && oDRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   if(oDCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Dir_getPath(oDCurr))+1;

      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Dir_getPath(oDCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Dir_T oDNewDir = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oDFirstNew != NULL)
            (void) Dir_free(oDFirstNew);
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Dir_new(oPPrefix, oDCurr, &oDNewDir);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oDFirstNew != NULL)
            (void) Dir_free(oDFirstNew);
         return iStatus;
      }

      /* set up for next level */
      Path_free(oPPrefix);
      oDCurr = oDNewDir;
      ulNewDirs++;
      if(oDFirstNew == NULL)
         oDFirstNew = oDCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oDRoot == NULL)
      oDRoot = oDFirstNew;
   ulCount += ulNewDirs;
   if (oDEnd != NULL) {
        oDEnd = &oDCurr;
   }
   return SUCCESS;
}
/*--------------------------------------------------------------------*/


int FT_insertDir(const char *pcPath) {
   int iStatus;
   Path_T oPPath = NULL;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

    return FT_insertPath(oPPath, NULL);
}

boolean FT_containsDir(const char *pcPath) {
   int iStatus;
   Dir_T oDFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findDir(pcPath, &oDFound);
   return (boolean) (iStatus == SUCCESS);
}

int FT_rmDir(const char *pcPath) {
   int iStatus;
   Dir_T oDFound = NULL;

   assert(pcPath != NULL);

   iStatus = FT_findDir(pcPath, &oDFound);

   if(iStatus != SUCCESS)
       return iStatus;

   ulCount -= Dir_free(oDFound);
   if(ulCount == 0)
      oDRoot = NULL;

   return SUCCESS;
}

/* This is a static method that takes in a pcPath string
corresponding to the absolute path of a file. return 
the absolute path of the directory containing it  */
static int FT_getPrevDir(const char *pcPath, Dir_T *oDDir, Path_T *oPPrevDir) {
    int iStatus;
    size_t ulDepth;
    Path_T oPPath;
    assert(pcPath != NULL);
   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
     return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

    ulDepth = Path_getDepth(oPPath);
    iStatus = Path_prefix(oPPath, ulDepth-1, oPPrevDir);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    iStatus = FT_findDir(Path_getPathname(*oPPrevDir), oDDir);
    return iStatus;
}


int FT_insertFile(const char *pcPath, void *pvContents, size_t ulLength) {
    int iStatus;
    size_t ulDepth;
    size_t ulIdx;
    Path_T oPPath;
    Path_T oPPrevDir;
    Dir_T oDEnd;
    File_T oFFile;

    assert(pcPath != NULL);
   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;


    /* insert directory right before the file */
    ulDepth = Path_getDepth(oPPath);
    if (ulDepth == 1) {
      return CONFLICTING_PATH;
    }
    iStatus = Path_prefix(oPPath, ulDepth-1, &oPPrevDir);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    iStatus = FT_insertPath(oPPrevDir, &oDEnd);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    /* now, insert file if not already in tree */
    if (Dir_hasDirChild(oDEnd, oPPath, &ulIdx)) {
        return ALREADY_IN_TREE;
    } 
    if (Dir_hasFileChild(oDEnd, oPPath, &ulIdx)) {
        return ALREADY_IN_TREE;
    }
    iStatus = File_new(oPPath, oDEnd, pvContents, &oFFile);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    iStatus = Dir_addFileChild(oDEnd, oFFile, ulIdx);
    return iStatus;
}

boolean FT_containsFile(const char *pcPath) {
    int iStatus;
    size_t ulIdx;
    Dir_T oDEnd;
    Path_T oPPath;
    boolean result;

    iStatus = FT_getPrevDir(pcPath, &oDEnd, &oPPath);
    if (iStatus != SUCCESS) {
        return FALSE;
    }
    result = Dir_hasFileChild(oDEnd, oPPath, &ulIdx);
    return result;
}

int FT_rmFile(const char *pcPath) {
    int iStatus;
    size_t ulIdx;
    Path_T oPPrevDir;
    Dir_T oDEnd;

    iStatus = FT_getPrevDir(pcPath, &oDEnd, &oPPrevDir);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    if (Dir_hasDirChild(oDEnd, oPPrevDir, &ulIdx)) {
        return NOT_A_FILE;
    }
    if (!Dir_hasFileChild(oDEnd, oPPrevDir, &ulIdx)) {
        return NO_SUCH_PATH;
    }
    ulCount -= Dir_freeFile(oDEnd, ulIdx); 
    return SUCCESS;
}

void *FT_getFileContents(const char *pcPath) {
    int iStatus;
    size_t ulFileIdx;
    File_T oFFile;
    Path_T oPPrevDir;
    Dir_T oDEnd;
    void *result;

    iStatus = FT_getPrevDir(pcPath, &oDEnd, &oPPrevDir);
    if (iStatus != SUCCESS) {
        return NULL;
    }

    Dir_hasFileChild(oDEnd, oPPrevDir, &ulFileIdx);
    iStatus = Dir_getFileChild(oDEnd, ulFileIdx, &oFFile);
    if (iStatus != SUCCESS) {
        return NULL;
    }
    result =  File_getContents(oFFile);
    return result;
}

void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {
    int iStatus;
    size_t ulFileIdx;
    File_T oFFile;
    Path_T oPPrevDir;
    Dir_T oDEnd;
    void *result;

    iStatus = FT_getPrevDir(pcPath, &oDEnd, &oPPrevDir);
    if (iStatus != SUCCESS) {
        return NULL;
    }

    Dir_hasFileChild(oDEnd, oPPrevDir, &ulFileIdx);
    iStatus = Dir_getFileChild(oDEnd, ulFileIdx, &oFFile);
    if (iStatus != SUCCESS) {
        return NULL;
    }
    result = File_replaceContents(oFFile, pvNewContents, ulNewLength);
    return result;
}

int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
    int iStatus;
    size_t ulIdx;
    File_T oFFile;
    Path_T oPPrevDir;
    Dir_T oDEnd;

    iStatus = FT_getPrevDir(pcPath, &oDEnd, &oPPrevDir);
    if (iStatus != SUCCESS) {
        return iStatus;
    }
    if (Dir_hasDirChild(oDEnd, oPPrevDir, &ulIdx)) {
        *pbIsFile = FALSE;
        return SUCCESS;   
    }
    if (Dir_hasFileChild(oDEnd, oPPrevDir, &ulIdx)) {
        Dir_getFileChild(oDEnd, ulIdx, &oFFile);
        *pbIsFile = TRUE;
        *pulSize = File_getContentSize(oFFile);
        return SUCCESS;
    } 
    return NO_SUCH_PATH;
}

int FT_init(void) {
   if(bIsInitialized)
      return INITIALIZATION_ERROR;

   bIsInitialized = TRUE;
   oDRoot = NULL;
   ulCount = 0;

   return SUCCESS;
}

int FT_destroy(void) {
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   if(oDRoot) {
      ulCount -= Dir_free(oDRoot);
      oDRoot = NULL;
   }

   bIsInitialized = FALSE;
   return SUCCESS;
}

/* --------------------------------------------------------------------

  The following auxiliary functions are used for generating the
  string representation of the DT.
*/

/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
*/
static size_t FT_preOrderTraversal(Dir_T n, DynArray_T d, size_t i) {
   size_t c;

   assert(d != NULL);

   if(n != NULL) {
      (void) DynArray_set(d, i, n);
      i++;
      for(c = 0; c < Dir_getNumDirChildren(n); c++) {
         int iStatus;
         Dir_T oDChild = NULL;
         iStatus = Dir_getDirChild(n,c, &oDChild);
         assert(iStatus == SUCCESS);
         i = FT_preOrderTraversal(oDChild, d, i);
      }
   }
   return i;
}

/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(Dir_T oDDir, size_t *pulAcc) {
    size_t c;
    File_T oFFile;
   assert(pulAcc != NULL);

   if(oDDir != NULL)
      *pulAcc += (Path_getStrLength(Dir_getPath(oDDir)) + 1);
    for (c = 0; c < Dir_getNumFileChildren(oDDir); c++) {
      Dir_getFileChild(oDDir, c, &oFFile);
        *pulAcc += (Path_getStrLength(File_getPath(oFFile)) + 1);
    } 
}

/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(Dir_T oDDir, char *pcAcc) {
  size_t c;
  File_T oFFile;
  assert(pcAcc != NULL);

   if(oDDir != NULL) {
        for (c = 0; c < Dir_getNumFileChildren(oDDir); c++) {
            Dir_getFileChild(oDDir, c, &oFFile);
            strcat(pcAcc, Path_getPathname(File_getPath(oFFile)));
            strcat(pcAcc, "\n");
      } 
      strcat(pcAcc, Path_getPathname(Dir_getPath(oDDir)));
      strcat(pcAcc, "\n");
   }
}
/*--------------------------------------------------------------------*/

char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char *result = NULL;

   if(!bIsInitialized)
      return NULL;

   nodes = DynArray_new(ulCount);
   (void) FT_preOrderTraversal(oDRoot, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);

   return result;
}
