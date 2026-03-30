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
   Node_T oNFirstNew = NULL;
   Dir_T oDCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;

   assert(pcPath != NULL);

   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;

   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;

    ulDepth = Path_getDepth(oPPath);
    ulIndex = 0;
    oDCurr = oDRoot;
    while (ulIndex < ulDepth) {
        Path_T oPPrefix = NULL;
        Dir_T oDNew = NULL;
        iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
        if (iStatus != SUCCESS) {
            Path_free(oPPath);
            return iStatus;
        }
        if (oDCurr == NULL) {
            oDRoot = Dir_new(oPPrefix, oDCurr);
            if (oDRoot == NULL) {
                return MEMORY_ERROR;
            }
            ulNewNodes++;
        } else {
            if (Dir_hasFileChild(oDCurr, oPPrefix)) {
                return CONFLICTING_PATH;
            }
            if (!Dir_hasDirChild(oDCurr, oPPrefix)) {
                oDNew = Dir_new(oPPrefix, oDCurr);
                if (oDNew == NULL) {
                    return MEMORY_ERROR;
                }
                Dir_insertDir(oDCurr, ODnew);
                ulNewNodes++;
            }
        }
        ulIndex++;
        oDCurr = oDnew;
    }

   /* find the closest ancestor of oPPath already in the tree */
   iStatus= DT_traversePath(oPPath, &oNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }

   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }

   ulDepth = Path_getDepth(oPPath);
   if(oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;

      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }

   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;

      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }

      /* insert the new node for this level */
      iStatus = Node_new(oPPrefix, oNCurr, &oNNewNode);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }
      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }

   Path_free(oPPath);
   /* update DT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;

   assert(CheckerDT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}