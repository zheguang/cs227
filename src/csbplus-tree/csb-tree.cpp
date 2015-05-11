/*   
  Hybrid Memory Cache-Sensitive B+-Tree code.                                            
    written by Harsha Yeddanapudy, William Truong, and Andrew Osgood (cs.brown.edu)
    May 10, 2015
  
    Code based on paper written by Rao and Ross (Columbia). Thank you to 
    Professor Stan Zdonik, our PhD mentor Sam Zhao, and Jun Rao (from whom
    we based our code off of). 
*/
/******************************************************************************/
/*   Cache-Sensitive B+-Tree code.                                            */
/*    Written by Jun Rao (junr@cs.columbia.edu)                               */
/*    Sep. 25, 1999                                                           */
/*                                                                            */
/*   This code is copyrighted by the Trustees of Columbia University in the   */
/*   City of New York.                                                        */
/*                                                                            */
/*   Permission to use and modify this code for noncommercial purposes is     */
/*   granted provided these headers are included unchanged.                   */
/******************************************************************************/

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <tgmath.h>
#include "csb-tree.hpp"
#include <sstream>
#include <string>

using namespace hmindex;
using std::size_t;
using std::cerr;
using std::runtime_error;
using std::string;

//===========================================================
//    B+Tree PUBLIC FUNCTIONS
//===========================================================

bp_t::~bp_t() {
	if (g_root == NULL) return;
	remove(g_root, sizeof(g_root));
	g_root = NULL;
	return;
}

// Checks if the given node is a leaf
// returns 1 if leaf 0 if not leaf
bool bp_t::IsLeaf(void* x){
	if (!((static_cast<CSBLNODE *>(x)->d_flag))) {
    return 1;
  } else {
    return 0;
  }
}


// frees a node in memory
// Inputs: node - the node to free
//         size - the size of the object to free
void bp_t::free_node(void* node, int size) {
    try{ 
        HybridMemory::assertAddress(node, HybridMemory::DRAM);
        HybridMemory::free(node, size, HybridMemory::DRAM);
    } catch (FatalError& err) {
        try {
            HybridMemory::assertAddress(node, HybridMemory::NVM);
            HybridMemory::free(node, size, HybridMemory::NVM);
        } catch (FatalError& err) {
            cout << "Encountered invalid memory type " << node << "\n";
        }
    } 
}

// removes a node from the csb tree
// Inputs: the node to free
//         the size of the node being freed
void bp_t::remove(void* node, int size) {  
    if (node == NULL) return;

    if (node == g_root) {
        g_root = NULL;
        free_node(node, size);
        return;
    } else {
        free_node(node, size);
    }
    return;
}

// Since a table typically grows rather than shrinks, we implement the lazy version of delete.
// Instead of maintaining the minimum occupancy, we simply locate the key on the leaves and delete it.
// return 1 if the entry is deleted, otherwise return 0.
//Inputs: the root of the CSB+ tree
//	  the leaf pair entry that is desired for deletion
int bp_t::csbdelete(CSBINODE* root, LPair del_entry) {
  int l,h,m, i;

  while (!IsLeaf(root)) {
    l=0;
    h=root->d_num-1;
    while (l<=h) {
      m=(l+h)>>1;
      if (del_entry.d_key <= root->d_keyList[m])
        h=m-1;
      else
        l=m+1;
    }
    root = (CSBINODE*) root->d_firstChild+l;
  }

  //now search the leaf
  l=0;
  h= ((CSBLNODE*)root)->d_num-1;
  while (l<=h) {
    m=(l+h)>>1;
    if (del_entry.d_key <= ((CSBLNODE*)root)->d_entry[m].d_key)
      h=m-1;
    else
      l=m+1;
  }

  do {
    while (l<((CSBLNODE*)root)->d_num) {
      if (del_entry.d_key==((CSBLNODE*)root)->d_entry[l].d_key) {
        if (del_entry.d_tid == ((CSBLNODE*)root)->d_entry[l].d_tid) { //delete this entry
          //we've hit the entry, so loop through entry list and update pointers
          //to the next one
          
          for (i=l; i<((CSBLNODE*)root)->d_num-1; i++)
            ((CSBLNODE*)root)->d_entry[i]=((CSBLNODE*)root)->d_entry[i+1];
          ((CSBLNODE*)root)->d_num--;

          //free memory alocation for tuple we just found?
        return 1;
        }
      l++;
      }
      else
        return 0;
    }
      root=(CSBINODE*) ((CSBLNODE*)root)->d_next;
      l=0;
  } while (root);
  return 0;
}

// initializes an internal node, pass in current depth to put in proper space in memory
// for inserting BY_HEIGHT
// Inputs: size - the size of the internal node being created;
//         isRoot - a 0 or 1 depending on if the node being created wll be the root
//	   currDepth - the current depth of the tree that this node is being allocated in
bp_t::CSBINODE* bp_t::init_internal(int size, int isRoot, int currDepth)
{
    /* find out what type of memory to allocate node in*/
    int memoryNodeType;
    if (isRoot)
      memoryNodeType = shouldbe_inmemory(NULL, 0, currDepth);
    else
      memoryNodeType = shouldbe_inmemory(g_root, 0, currDepth);
    
    HybridMemory::MEMORY_NODE_TYPE test;
    if (memoryNodeType)
      test = HybridMemory::DRAM;
    else
      test = HybridMemory::NVM;

    CSBINODE * np = static_cast<CSBINODE *>(HybridMemory::alloc(sizeof(CSBINODE)*size, test));

    for (int i = 0; i < size; ++i)
    { 
        CSBINODE * temp = np + i;
        temp->d_num = 0;
        temp->d_firstChild = NULL;
    }

    return np;
}

// initializes a leaf node
// Inputs: size - the size of the node being created
bp_t::CSBLNODE* bp_t::init_leaf(int size)
{
    /* find out what type of memory to allocate node in*/
    //pass in -1 for current depth since this is a leaf node, 
    int mem = shouldbe_inmemory(g_root, 1, -2);

    HybridMemory::MEMORY_NODE_TYPE test;
    if (mem)
      test = HybridMemory::DRAM;
    else
      test = HybridMemory::NVM;

    CSBLNODE * np = static_cast<CSBLNODE *>(HybridMemory::alloc(sizeof(CSBLNODE)*size, test));

    for (int i = 0; i < size; ++i)
    {
        CSBLNODE * temp = np + i;
        temp->d_num = 0;
        temp->d_prev = NULL;
        temp->d_next = NULL;
    }

    return np;
}

// searches internal node based on given key
// does not use a binary search, iterates through whole keylist instead
// Inputs: node - internal node we are searching
// key - key are looking for
int bp_t::internal_search(CSBINODE *node, int key) {
  int length = node->d_num;
  int i;

  //case for end of list
  if (node->d_keyList[length-1] < key) {
    return length;
  }
  if (node-> d_keyList[0] >= key) { 
    return 0;
  }

  for (i = 1; i < length; i++) {
    if (node->d_keyList[i] == key) {
      return i;
    }
    else if (key < node->d_keyList[i]) {
      return i;
    } 
  }
  return -1;
}

// searches leaf node for a key
// uses binary search to find key
// Inputs: node - leaf node we are searching
// key - key are looking for
int bp_t::leaf_search(CSBLNODE *node, int key) {
    int start = 0;
    int end = node->d_num-1;
    while (end - start > 1) {
        int middle = (start + end) / 2;
        if (key <= node->d_entry[middle].d_key) {
            end = middle;
        } else if ( key > node->d_entry[middle].d_key) {
            start = middle;
        } 
    }
    if (key >= node->d_entry[end].d_key) {
        return end;
    }
    return start;
}

// searches entire tree for a given key
// Inputs: root -  the root node of the tree
//         key - the key value to search for
int bp_t::search(CSBINODE* root, int key) {
    while (!IsLeaf(root)) {
        int offset = internal_search(root, key);
        root=(CSBINODE*) root->d_firstChild+offset;
    }

    int l = leaf_search((CSBLNODE*) root, key);

    if (l<((CSBLNODE*)root)->d_num && key==((CSBLNODE*)root)->d_entry[l].d_key)
        return ((CSBLNODE*)root)->d_entry[l].d_tid;
    else
        return 0;
}

// function to initialize csb tree before inserts
// naive version of tiling
// Inputs: root - the root of the tree
void bp_t::addChildToRoot(CSBINODE* root) {
  CSBLNODE * newChild = init_leaf(1);
  root->d_firstChild = newChild;
  root->d_num = 1;
  root->d_keyList[0] = 5;
  g_height = 1;
}

// bulk load a CSB+-Tree 
// Inputs:
// n: size of the sorted array
// a: sorted leaf array
// iUpper: maximum number of keys for each internal node duing bulkload. = 11
// lUpper: maximum number of keys for each leaf node duing bulkload. = 3
// Note: iUpper has to be less than keysize
//       lUpper has to be less than leafsize
void bp_t::bulkload(int n, LPair* a, int iUpper, int lUpper) {
  CSBLNODE *lcurr, *start, *lprev;
  CSBINODE *iLow, *iHigh, *iHighStart; 
  int i, j, nLeaf, nHigh, nLow, remainder;
  // first step, populate all the leaf nodes 
  nLeaf=(n+lUpper-1)/lUpper; // = 334 leaf nodes for inserting 1000
  lcurr = init_leaf(nLeaf);
  g_height++;
  lcurr->d_flag=0;
  lcurr->d_num=0;
  lcurr->d_prev=0;
  start=lcurr;

  for (i=0; i<n; i++) {
    if (lcurr->d_num >= lUpper) { // at the beginning of a new node 
      lprev=lcurr;
      lcurr++;
      lcurr->d_flag=0;
      lcurr->d_num=0;
      lcurr->d_prev=lprev;
      lprev->d_next=lcurr;
    }
    lcurr->d_entry[lcurr->d_num]=a[i];
    lcurr->d_num++;
  }
  lcurr->d_next=0;
  
  // second step, build the internal nodes, level by level.
  // we can put IUpper keys and IUpper+1 children (implicit) per node
  nHigh=(nLeaf+iUpper)/(iUpper+1); // = 28 
  remainder=nLeaf%(iUpper+1); // 10
  //currDepth is -1 since we don't need to check for depth on bulk insert
  iHigh = init_internal(nHigh, 0, -2);
  g_height++;
  iHigh->d_num=0;
  iHigh->d_firstChild=start;
  iHighStart=iHigh;
  lcurr=start;
  for (i=0; i<((remainder==0)?nHigh:(nHigh-1)); i++) {
    iHigh->d_num=iUpper;
    iHigh->d_firstChild=lcurr;
    for (j=0; j<iUpper+1; j++) {
      iHigh->d_keyList[j]=lcurr->d_entry[lcurr->d_num-1].d_key;
      lcurr++;
    }
    iHigh++;
  }
  if (remainder==1) {
    //this is a special case, we have to borrow a key from the left node if there is one
    //leaf node remaining.
    iHigh->d_keyList[0]=(iHigh-1)->d_keyList[iUpper];
    (iHigh-1)->d_num--;
    iHigh->d_num=1;
    iHigh->d_firstChild=lcurr-1;
    lcurr++;
  }
  else if (remainder>1) {
    iHigh->d_firstChild=lcurr;
    for (i=0; i<remainder; i++) {
      iHigh->d_keyList[i]=lcurr->d_entry[lcurr->d_num-1].d_key;
      lcurr++;
    }
    iHigh->d_num=remainder-1; 
  }
  while (nHigh>1) {
    nLow=nHigh;
    iLow=iHighStart;
    nHigh=(nLow+iUpper)/(iUpper+1);
    remainder=nLow%(iUpper+1);
    iHigh = init_internal(nHigh, 0, -2);
    g_height++;
    iHigh->d_num=0;
    iHigh->d_firstChild=iLow;
    iHighStart=iHigh;

    for (i=0; i<((remainder==0)?nHigh:(nHigh-1)); i++) {
      iHigh->d_num=iUpper;
      iHigh->d_firstChild=iLow;
      for (j=0; j<iUpper+1; j++) {//iUpper+1 = 12
        iHigh->d_keyList[j]=iLow->d_keyList[iLow->d_num];
        iLow++;
      }
      iHigh++;
    }

    if (remainder==1) { //this is a special case, we have to borrow a key from the left node
      iHigh->d_keyList[0]=(iHigh-1)->d_keyList[iUpper];
      (iHigh-1)->d_num--;
      iHigh->d_num=1;
      iHigh->d_firstChild=iLow-1;
      iLow++;  
    }
    else if (remainder>1) {
      iHigh->d_firstChild=iLow;
      for (i=0; i<remainder; i++) {
        assert(iLow->d_num<14);
	iHigh->d_keyList[i]=iLow->d_keyList[iLow->d_num];
        iLow++;
      }
      iHigh->d_num=remainder-1;
    }
  }
  

  g_root=iHighStart;
}
// A function that inserts new nodes into the CSB+ Tree
// Inputs:
// root: the current node group being looked at for insertion
// parent: the parent of the current node
// int childIndex; the offset that delineates which node in the node group is the node to examine
// new_entry: the new entry being inserted
// new_key: the key within the tree that will be inserted
// new_child: the new_child being placed with the root after a split
// currDepth: the current depth within the tree
void bp_t::insert(CSBINODE* root, CSBINODE* parent, int childIndex, LPair new_entry, int* new_key, void** new_child, int currDepth) {
      int l,h,m,i,j;
      // cout << "currDepth = " << currDepth << "\n";
        if (IsLeaf(root)) {    // This is a leaf node
            l=0;
            h=((CSBLNODE*)root)->d_num-1;
            while (l<=h) {
                m=(l+h)>>1;
                if (new_entry.d_key <= ((CSBLNODE*)root)->d_entry[m].d_key)
                    h=m-1;
                else
                    l=m+1;
            }
            // l can range from 0 to ((BPLNODE *)root)->d_num
            // insert entry at the lth position, move everything from l to the right.
            if (((CSBLNODE*)root)->d_num < config.leafsize) {   //we still have enough space in this leaf node.
                for (i=((CSBLNODE*)root)->d_num; i>l; i--) {
                    ((CSBLNODE*)root)->d_entry[i]=((CSBLNODE*)root)->d_entry[i-1];
                }
                ((CSBLNODE*)root)->d_entry[l]=new_entry;
                ((CSBLNODE*)root)->d_num++;
                *new_child=0;
            } else { // we have to split this leaf node
                CSBLNODE *new_lnode, *old_lnode;
                CSBLNODE *new_group, *old_group;
      
                old_group=(CSBLNODE*) parent->d_firstChild;
                if (parent->d_num < config.keysize) { // we don't have to split the parent
                    new_group = init_leaf(parent->d_num+2);
                     
                      for (i=0; i<=childIndex; i++) 
                        new_group[i]=old_group[i];
                      for (i=childIndex+2; i<=parent->d_num+1; i++) 
                        new_group[i]=old_group[i-1];
                      new_group[0].d_prev=old_group[0].d_prev;
                      for (i=1; i<=parent->d_num+1; i++) {
                        new_group[i].d_prev=new_group+i-1;
                        new_group[i-1].d_next=new_group+i;
                      }
                      new_group[parent->d_num+1].d_next=old_group[parent->d_num].d_next;
                      if (new_group[parent->d_num+1].d_next)
                        (new_group[parent->d_num+1].d_next)->d_prev=new_group+parent->d_num+1;
                      if (new_group[0].d_prev)
                        (new_group[0].d_prev)->d_next=new_group;
                      old_lnode=new_group+childIndex;
                      new_lnode=new_group+childIndex+1;
                     
                      remove(old_group, sizeof(CSBLNODE)*parent->d_num);

                } else { // we also have to split parent. We have 15+1 nodes, put 8 in each node group.
                      new_group = init_leaf((config.keysize/2) + 1);

                       if (childIndex >= (config.keysize / 2)) { // the new node (childIndex+1) belongs to new group
                        for (i=(config.keysize / 2), j=config.keysize; i>=0; i--) { 
                          if (i != childIndex-(config.keysize / 2)) {
                            new_group[i]=old_group[j];
                            j--;
                          }
                        }
                        if (childIndex==config.keysize)    //the new node is the last one
                          new_group[(config.keysize / 2)].d_next=old_group[config.keysize].d_next;
                      }
                      else { //the new node belongs to the old group
                        for (i=(config.keysize / 2); i>=0; i--) 
                          new_group[i]=old_group[i+(config.keysize / 2)];
                        for (i=(config.keysize / 2); i>childIndex+1; i--)
                          old_group[i]=old_group[i-1];
                      }
                      new_group[0].d_prev=old_group+(config.keysize / 2);
                      for (i=1; i<=(config.keysize / 2); i++) {
                        new_group[i].d_prev=new_group+i-1;
                        new_group[i-1].d_next=new_group+i;
                        old_group[i].d_prev=old_group+i-1;
                        old_group[i-1].d_next=old_group+i;
                      }
                      new_group[(config.keysize / 2)].d_next=old_group[config.keysize].d_next;
                      old_group[(config.keysize / 2)].d_next=new_group;
                      if (new_group[(config.keysize / 2)].d_next)
                        (new_group[(config.keysize / 2)].d_next)->d_prev=new_group+(config.keysize / 2);

                      if ((childIndex+1)>=((config.keysize / 2)+1)) {
                        new_lnode = new_group+childIndex-(config.keysize / 2);
                      } else {
                        new_lnode = old_group+childIndex+1;
                      }
                      if ((childIndex)>=((config.keysize / 2)+1)) {
                        old_lnode = new_group+childIndex-(config.keysize / 2)-1;
                      } else {
                        old_lnode = old_group+childIndex;
                      }

                      //ADDED THIS, CHECK IF RIGHT
                      remove(old_group + ((config.keysize / 2) + 1), sizeof(CSBLNODE)*(config.keysize / 2));
                      for (i=(config.keysize / 2)+1; i<config.keysize+1; i++)
                        new_group[i].d_num=-1;
                      for (i=(config.keysize / 2)+1; i<config.keysize+1; i++)
                        old_group[i].d_num=-1;
                }

                if (l > (config.leafsize / 2)) { //entry should be put in the new node
                  for (i=(config.leafsize / 2)-1, j=config.leafsize-1; i>=0; i--) {
                    if (i == l-(config.leafsize / 2)-1) {
                      new_lnode->d_entry[i]=new_entry;
                    }
                    else {

                      new_lnode->d_entry[i]=old_lnode->d_entry[j];
                      j--;
                    }
                  }
                }
                else { //entry should be put in the original node
                  for (i=(config.leafsize / 2)-1; i>=0; i--) 
                    new_lnode->d_entry[i]=old_lnode->d_entry[i+(config.leafsize / 2)];
                  for (i=(config.leafsize / 2); i>l; i--) 
                    old_lnode->d_entry[i]=old_lnode->d_entry[i-1];
                    old_lnode->d_entry[l]=new_entry;
                }
                  new_lnode->d_num=(config.leafsize / 2);
                  new_lnode->d_flag=0;
                  old_lnode->d_num=(config.leafsize / 2)+1;
                  *new_key=old_lnode->d_entry[(config.leafsize / 2)].d_key;
                  *new_child=new_group;
            }
        }
        else {  //this is an internal node
            l=0;
            h=root->d_num-1;
            while (l<=h) {
                m=(l+h)>>1;
                if (new_entry.d_key <= root->d_keyList[m])
                  h=m-1;
                else
                  l=m+1;
            }
            int nextDepth = currDepth - 1;
            // cout << currDepth;
            insert(((CSBINODE*)root->d_firstChild)+l, root, l, new_entry, new_key, new_child, nextDepth);
            if (*new_child) {
              if (root->d_num<config.keysize) { // insert the key right here, no further split
                    for (i=root->d_num; i>l; i--) {
                        root->d_keyList[i]=root->d_keyList[i-1];
                    }
                    root->d_keyList[i]=*new_key;
                    root->d_firstChild=*new_child;  // *new_child represents the pointer to the new node group
                    root->d_num++;
                    *new_child=0;
              }
              else {  // we have to split again
                CSBINODE *new_node, *old_node, *new_group;

                if (parent==0) { // now, we need to create a new root 
                    g_root = init_internal(1, 0, currDepth);

                    new_group= init_internal(2, 0, currDepth);

                    g_height++;
                    // cout << g_height << "\n";

                    new_group[0]=*root;
                    old_node=new_group;
                    new_node=new_group+1;
                    g_root->d_num=1;
                    g_root->d_firstChild=new_group;
                    // make delete
                    remove(root, sizeof(CSBINODE)*1);
                }
                else { //there is a parent
                    CSBINODE *old_group;
    
                    old_group=(CSBINODE*) parent->d_firstChild;
                    if (parent->d_num < config.keysize) { // no need to split the parent
                      // same here, now the new node group has parent->d_num+2 nodes.
                        new_group = init_internal(parent->d_num+2, 0, currDepth);
                          for (i=0; i<=childIndex; i++) 
                            new_group[i]=old_group[i];
                          for (i=childIndex+2; i<=parent->d_num+1; i++) 
                            new_group[i]=old_group[i-1];
                          old_node=new_group+childIndex;
                          new_node=new_group+childIndex+1;
                          
                          //change to proper delete
                          remove(old_group, sizeof(CSBINODE)*parent->d_num);
                }
                else { // we also have to split parent. We have 15+1 nodes, put 8 nodes in each group.
                    new_group = init_internal((config.keysize / 2) + 1, 0, currDepth);
                       if (childIndex >= (config.keysize / 2)) { // the new node belongs to new group
                            for (i=(config.keysize / 2), j=config.keysize; i>=0; i--) {
                                if (i != childIndex-(config.keysize / 2)) {
                                  new_group[i]=old_group[j];
                                  j--;
                                }
                              } 
                          }
                            else { //the new node belongs to the old group
                                for (i=(config.keysize / 2); i>=0; i--) 
                                    new_group[i]=old_group[i+(config.keysize / 2)];
                                for (i=(config.keysize / 2); i>childIndex+1; i--)
                                    old_group[i]=old_group[i-1];
                            }
                              
                              if ((childIndex+1)>=((config.keysize / 2)+1)) {
                                new_node= new_group+childIndex-(config.keysize / 2);
                              } else {
                                new_node= old_group+childIndex+1;
                              }
                                
                              if (((childIndex)>=(config.keysize / 2)+1)) {
                                old_node= new_group+childIndex-(config.keysize / 2)-1;
                              } else {
                                old_node= old_group+childIndex;
                              }

                              remove(old_group + (config.keysize / 2) + 1, sizeof(CSBLNODE)*(config.keysize / 2));

                              for (i=(config.keysize / 2) + 1; i<config.keysize + 1; i++)
                                new_group[i].d_num=-1;
                              for (i=(config.keysize / 2) + 1; i<config.keysize + 1; i++)
                                old_group[i].d_num=-1;
                }
            }
              // the largest key in old_node is then promoted to the parent. 
              if (l > (config.keysize / 2)) {     // new_key to be inserted in the new_node
                for (i=(config.keysize / 2)-1, j=config.keysize-1; i>=0; i--) {
                  if (i == l-(config.keysize / 2)-1)
                    new_node->d_keyList[i]=*new_key;
                  else {
                    new_node->d_keyList[i]=old_node->d_keyList[j];
                    j--;
                  }
                }
              }
              else {    // new_key to be inserted in the old_node
                for (i=(config.keysize / 2)-1; i>=0; i--) 
                  new_node->d_keyList[i]=old_node->d_keyList[i+(config.keysize / 2)];
                for (i=(config.keysize / 2); i>l; i--)
                  old_node->d_keyList[i]=old_node->d_keyList[i-1];
                old_node->d_keyList[l]=*new_key;
              }
              new_node->d_num=(config.keysize / 2);
              new_node->d_firstChild=*new_child;
              old_node->d_num=(config.keysize / 2);

              if (parent) 
                *new_key=old_node->d_keyList[(config.keysize / 2)];
              else
                g_root->d_keyList[0]=old_node->d_keyList[(config.keysize / 2)];

            *new_child=new_group;
      }
    }
  }
}

/* finds the height of the tree by traversing until it
finds a leaf node.  */
int bp_t::get_height(CSBINODE* root) {
  int level = 0;
  while (!IsLeaf(root)) {
      int offset = 0;
      root=(CSBINODE*) root->d_firstChild+offset;
      level++;
  }

  return level;
}

// checks the memory type (DRAM or NVM) for each level of the tree.
std::string bp_t::check_levels(CSBINODE* root) {
  std::string toReturn;
  toReturn += check_memory_type(root, sizeof(root));
  toReturn += "/";
  while (!IsLeaf(root)) {
      root=(CSBINODE*) root->d_firstChild;
      toReturn += check_memory_type(root, sizeof(root));
      toReturn += "/";
  }
  return toReturn;
}

// ************************ //
//    PRIVATE FUNCTIONS     //
// ************************ //
//Function that determines which type of memory should a node be placed in
// Inputs: root - the node to insert
//         isLeaf - whether or not the node is a leaf or not
//         currDepth - the current depth of the tree
int bp_t::shouldbe_inmemory(CSBINODE* root, int isLeaf, int currDepth) {
    int height = g_height;

    switch (config.policy) {
        case BY_HEIGHT:
            //this chunk is for bulkload
            if (currDepth == -2) {
              if (isLeaf) {
                return 0; //NVM
              }
              else {
                if (height < 2)
                  return 0;
                else
                  return 1; // if the height is less than
                // or equal to 2 then allocate in DRAM, else allocate in NVM
              }
            } else {
              // cout << "nope" << "\n";
              //this chunk is for insert
              if (currDepth < 2) //allocate to proper memory space based on node's current depth
                return 0;
              else
                return 1;
            }
        case BY_TIER_DRAM:
            if (isLeaf)
              return 1; //DRAM
            else
              if ((height % 2) == 0) {
                return 1;
              }
              else {
                return 0;
              }
        case BY_TIER_NVM:
            if (isLeaf)
              return 0; //NVM
            else
              if ((height % 2) == 0)
                return 0;
              else
                return 1;
        case DRAM:
          return 1;
        default:
            return 1;
    }
}

// checks which memory type this node is stored in,
// and returns 1 if DRAM and 0 if NVM (or null if 
// invalid memory type encountered)
char const* bp_t::check_memory_type(void* node, int size) {
    try{ 
        HybridMemory::assertAddress(node, HybridMemory::DRAM);
        return "DRAM";
    } catch (FatalError& err) {
        try {
            HybridMemory::assertAddress(node, HybridMemory::NVM);
            return "NVM";
        } catch (FatalError& err) {
            cout << "Encountered invalid memory type " << node << "\n";
            return "NEITHER";
        }
    } 
}
