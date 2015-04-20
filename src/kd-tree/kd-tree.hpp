#include <iostream>
#include <string>
#include "config.hpp"
#include "tuple.hpp"
using std::cout;

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
// TODO parameters to add: input ranking, fanout, and what?
class tree_t {
public:
	node_t* root;
	// Note, if one and only one of nvm_level and nvm_percentile
	// should be passed as input. If both are given valid values,
	// tree will use nvm_level
	tree_t(config_t& config) : 
			root(NULL), 
			config(config) {}

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	// Insert new node into tree
	void insert(node_t newnode);

  // Search nearest neighbor
	node_t* search_nearest(tuple_t& target) const;
	
	// Print the kd-tree
	void display() const; 

private:
	// Configuration of the hybrid memory allocation
	config_t config;
	
	int nvm_level = -1;
	int memory_depth = -1;

	void check_config(int num_points);

	bool inMemory(int h, int d) const;

	// Helper function for buildfrom
	node_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, node_t* parent) const;
	
	// Find parent node of tuple 
	// if it were to be inserted into tree rooted at starter
	node_t* find_parent(node_t* starter, tuple_t& tuple) const;
	
	void display_helper(node_t* node, string label) const;
 	
	node_t* search_nearest_helper(node_t* starter, tuple_t& target) const;
};
