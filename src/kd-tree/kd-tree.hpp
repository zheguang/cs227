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

// Node of kd-tree
struct node_t {
	tuple_t value;
	node_t* left;
	node_t* right;
	node_t* parent;
	int depth; // Depth of node in the tree
	node_t():left(NULL), right(NULL), parent(NULL), depth(0) {}	
};

// Class for kd-tree
// TODO generalize tree's fanout
class tree_t {
public:
	node_t* root;
	// Note, if one and only one of nvm_level and nvm_percentile
	// should be passed as input. If both are given valid values,
	// tree will use nvm_level
	tree_t(config_t& config) : 
			root(NULL), 
			config(config) {}

	~tree_t();

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	// Insert new node into tree
	void insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type);

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
	bool inMemory(int h, int d) const;

	// Helper function for buildfrom
	node_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, node_t* parent) const;
	
	// Find parent of tuple if it to be inserted into tree rooted at starter
	node_t* find_parent(
			node_t* starter, tuple_t& tuple, bool& is_left_child) const;
	
	// Helper function for display
	void display_helper(node_t* node, string label) const;
 	
	// Helper function for search_nearest
	node_t* search_nearest_helper(node_t* starter, tuple_t& target) const;

	// Helper function for destructor
	void free_tree_helper(node_t* node);

};
#endif
