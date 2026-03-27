/*--------------------------------------------------------------------*/
/* checkerDT.c                                                        */
/* Author:    Jeremy Arking and Ben Gitai                                                        */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"

/* for a node oNNode, returns FALSE if children array is NULL,
   its children are not sorted or in the leftmost entries of 
   the array. Otherwise return TRUE */
static boolean checkerDT_Children_isValid(Node_T oNNode) {
   Node_T oNChildCur;
   Node_T oNChildNext;
   size_t i;

   Node_getChild(oNNode, 0, &oNChildCur);
   if (oNChildCur == NULL && Node_getNumChildren(oNNode) > 0) {
      fprintf(stderr, "First child is NULL\n");
      return FALSE;
   }
   fprintf(stderr, "Node %s has %zu children\n", Path_getPathname(Node_getPath(oNNode)), Node_getNumChildren(oNNode));
   for(i = 1; i < Node_getNumChildren(oNNode) - 1; i++) {
      Node_getChild(oNNode, i, &oNChildNext);
      if (oNChildNext == NULL) {
         fprintf(stderr, "NULL child found at index %zu\noNChildNext output = %d", i, Node_getChild(oNNode, i, &oNChildNext));
         return FALSE;
      } else {
         if (Node_compare(oNChildCur, oNChildNext) > 0) {
            fprintf(stderr, "Directories must be in sorted order\n");
            return FALSE;
         }
      }
      oNChildCur = oNChildNext;
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   /* checks that the children are stored properly according to specification */
   if (!checkerDT_Children_isValid(oNNode)) {
      return FALSE;
   }

   return TRUE;
}

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerDT_treeCheck(Node_T oNNode) {
   size_t ulIndex;

   if(oNNode!= NULL) {

      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         int iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* Now checks invariants recursively at each node from the root. */
   return CheckerDT_treeCheck(oNRoot);
}
