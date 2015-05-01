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

const int MFKD_DEBUG = true;
const bool MFKD_KEY_SORTED = true;

// Node of kd-tree
struct node_t {
	int depth; // Depth of node in the tree
	int num_children;
	int childindex; // Number-th of child of its parent
	void* children;
	node_t* parent;
	vector<tuple_t> values;
};

// Class for kd-tree
class tree_t {
public:
	node_t* root;
	tree_t(config_t& config) : 
			root(NULL), 
			config(config) {
		nodesize = sizeof(node_t) +
				sizeof(datatype_t) * config.dimension * (config.fanout - 1);
	}

	~tree_t();
	
	void replace_node_value(node_t* replaced, int vindex);

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	node_t* insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type);

	void remove(node_t* node);

  // Search nearest neighbor
	node_t* search_nearest(tuple_t& target, datatype_t& sdist) const;
	
	// Print the kd-tree
	void display() const; 
	void print_node(node_t* node) const;

	int get_dimension() const {
		return config.dimension;
	}

private:
	// Configuration of the hybrid memory allocation
	config_t config;
	
	int nodesize;

	// Level of tree nodes from leaves should be resides in NVM
	int nvm_level = -1;
	
	// Depth of tree nodes from root should be resides in DRAM
	int memory_depth = -1;

	// Determine how to spread nodes across memory base
	void check_config(int num_points);

	// Check whether a node should be allocated in memory
	bool shouldbe_inmemory(int h, int d) const;
	
	// Insert newnode into children nodes of parent at childindex
	node_t* get_child(node_t* parent, int index) const;
	
	bool is_null(node_t* node) const;

	node_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, node_t* parent, int childindex) const;
	
	//XXX Find parent of tuple if it to be inserted into tree rooted at starter
	// is_left_child is true if tuple if inserted will be parent's left child
	node_t* find_parent(
			node_t* starter, tuple_t& tuple, int& willbe_child) const;
	
	void display_helper(node_t* node, string label) const;
 	
	node_t* search_nearest_helper(
			node_t* starter, tuple_t& target, datatype_t& sdist) const;

	// Helper function for destructor
	void free_tree_helper(node_t* start);
	void free_node(node_t* node);

//	node_t* find_replacement(node_t* replaced) const;
	node_t* find_largest(node_t* start, int comp_axis, int& index_smallest) const;
	node_t* find_smallest(node_t* start, int comp_axis, int& index_smallest) const;
	int index_of_smallest(node_t* node, int axis) const;
	int index_of_largest(node_t* node, int axis) const;
};
#endif
