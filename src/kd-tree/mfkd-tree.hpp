// @xl242
#ifndef KD_TREE_H_
#define KD_TREE_H_

#include <iostream>
#include <string>
#include "config.hpp"
#include "tuple.hpp"
#include "../HybridMemory.hpp"
using std::cout;
using namespace hmindex;

const int KD_DEBUG = false;
const bool KD_KEY_SORTED = true;

// Node of kd-tree
struct node_t {
	tuple_t value;
	vector<node_t*> children;
	node_t* parent;
	int depth; // Depth of node in the tree
	node_t(int fanout) : 
			parent(NULL),
			depth(0) {
		children.resize(fanout);
		for (int i = 0; i < fanout; i++) {
			children[i] = NULL;
		}
	}	
};

// Class for kd-tree
// TODO generalize tree's fanout
class tree_t {
public:
	node_t* root;
	tree_t(config_t& config) : 
			root(NULL), 
			config(config) {}

	~tree_t();

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	node_t* mf_insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type);

	void mf_remove(node_t* node);

  // Search nearest neighbor
	node_t* search_nearest(tuple_t& target) const;
	
	// Print the kd-tree
	void display() const; 

	int get_dimension() const {
		return config.dimension;
	}

private:
	// Configuration of the hybrid memory allocation
	config_t config;
	
	// Level of tree nodes from leaves should be resides in NVM
	int nvm_level = -1;
	
	// Depth of tree nodes from root should be resides in DRAM
	int memory_depth = -1;

	// Determine how to spread nodes across memory base
	void check_config(int num_points);

	// Check whether a node should be allocated in memory
	bool shouldbe_inmemory(int h, int d) const;

	node_t* mf_buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, node_t* parent) const;
	
	// Find parent of tuple if it to be inserted into tree rooted at starter
	// is_left_child is true if tuple if inserted will be parent's left child
	node_t* mf_find_parent(
			node_t* starter, tuple_t& tuple, int& willbe_child) const;
	
	void mf_display_helper(node_t* node, string label) const;
 	
	node_t* mf_search_nearest_helper(node_t* starter, tuple_t& target) const;

	// Helper function for destructor
	void free_tree_helper(node_t* start);
	void free_node(node_t* node);

	node_t* mf_find_replacement(node_t* replaced) const;
	node_t* mf_find_largest(node_t* start, int comp_axis) const;
	node_t* mf_find_smallest(node_t* start, int comp_axis) const;
};
#endif
