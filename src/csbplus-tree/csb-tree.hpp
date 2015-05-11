#ifndef BP_TREE_H_
#define BP_TREE_H_

#include <assert.h>
#include <cstdio>
#include <vector>
// #include <conio.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cstddef>
#include "config.hpp"
#include "../HybridMemory.hpp"
#include "../FatalError.hpp"

using namespace std;
using namespace hmindex;

// Class for csb+ tree
class bp_t {
public:
	struct LPair {
  		int d_key;
  		int d_tid;
	};

	/* csb+ tree leaf node */
	struct CSBLNODE {
	  int        d_num;       /* number of keys in the node */
	  void*      d_flag;      /* this pointer is always set to null and is used to distinguish
				     between and internal node and a leaf node */
	  CSBLNODE* d_prev;      /* backward and forward pointers */
	  CSBLNODE* d_next;
	  LPair d_entry[4];       /* <key, TID> pairs */
	};

	/* a CSB+-Tree internal node of 64 bytes.
	   corresponds to a cache line size of 64 bytes.
	   We put all the child nodes of any given node continuously in a node group and
	   store explicitly only the pointer to the first node in the node group.
	   We can store a maximum of 14 keys in each node.
	   Each node has a maximum of 15 implicit child nodes. 
	*/
	struct CSBINODE {
	  int    d_num;
	  void*  d_firstChild;       //pointer to the first child in a node group
	  int   d_keyList[12];
	public:
	  int operator == (const CSBINODE& node) {
	    if (d_num!=node.d_num)
	      return 0;
	    for (int i=0; i<d_num; i++)
	      if (d_keyList[i]!=node.d_keyList[i])
	        return 0;
	    return 1;
	  }
	};

	CSBINODE* g_root;
	int g_height;

	bp_t(config_t& config) : 
			g_root(init_internal(1, 1, 0)), 
			g_height(0),
			config(config) {
				// nothing
	};

	~bp_t();

	bool IsLeaf(void* x);

	void free_node(void* node, int size);

	void remove(void* node, int size);

	int csbdelete(CSBINODE* root, LPair del_entry);

	CSBINODE* init_internal(int size, int isRoot, int currDepth);

	CSBLNODE* init_leaf(int size);

	int internal_search(CSBINODE *node, int key);

	int leaf_search(CSBLNODE *node, int key);

	int search(CSBINODE* root, int key);

	void addChildToRoot(CSBINODE* root);

	void bulkload(int n, LPair* a, int iUpper, int lUpper);

	void insert(CSBINODE* root, CSBINODE* parent, int childIndex, LPair new_entry, int* new_key, void** new_child, int currDepth);

	std::string check_levels(CSBINODE* root);

	int get_height(CSBINODE* root);

private:
	// Configuration of the hybrid memory allocation
	config_t config;

	// Check where a node should be allocated in memory
	int shouldbe_inmemory(CSBINODE* root, int isLeaf, int currDepth);
	
	// testing function to check what memory type a node is in
	char const* check_memory_type(void* node, int size);
};
#endif
