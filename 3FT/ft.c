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
#include "nodeDT.h"
#include "checkerDT.h"
#include "dt.h"


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





int DT_insertDir(const char *pcPath) {
   int iStatus;
   Path_T oPPath = NULL;
   Dir_T oDCurr = NULL;
   Dir_T oDNew = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;
   Path_T oPPrefix = NULL;
   Dir_T oDNew = NULL;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

    ulDepth = Path_getDepth(oPPath);
    ulIndex = 1;
    oDCurr = oDRoot;

    iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
    if (iStatus != SUCCESS) {
        Path_free(oPPath);
        return iStatus;
    }
    /* handle the case where there is not a root already*/
    if (oDRoot == NULL) {
        oDRoot = Dir_new(oPPrefix, NULL);
        if (oDRoot == NULL) {
            return MEMORY_ERROR;
        }
        ulNewNodes++;
        ulIndex++;
    }
    /* handle the case where root does not match up to the root of the path */
    if (Path_comparePath(oDRoot->oPPath, oPPrefix) != 0) {
        return CONFLICTING_PATH;
    }

    /* otherwise traverse through file tree adding dirs as needed */
    while (ulIndex <= ulDepth) {
        iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
        if (iStatus != SUCCESS) {
            Path_free(oPPath);
            return iStatus;
        }
        if (Dir_hasFileChild(oDCurr, oPPrefix)) {
            return NOT_A_Dir;
        }
        if (Dir_hasDirChild(oDCurr, oPPrefix)) {
            if (ulIndex == ulDepth) {
                return ALREADY_IN_TREE;
            }
        } else {
            oDNew = Dir_new(oPPrefix, oDCurr);
            if (oDNew == NULL) {
                return MEMORY_ERROR;
            }
            Dir_insertDir(oDCurr, ODnew);
            ulNewNodes++;
        }
        ulIndex++;
        oDCurr = oDnew;
    }
    return SUCCESS;
}