#include <iostream>
#include "tuple.hpp"
using std::cout;
using std::string;
using std::vector;
using std::ostringstream;

const bool CSKD_DEBUG = true;

struct cs_node_t {
	tuple_t value;
	void* children;
	int depth; // Depth of node in the tree
};

class cs_tree_t {
public:
	cs_node_t* root;
	cs_tree_t(int d, int nvm_level=0, fanout=2) : 
			root(NULL), 
			dimension(d),
			fanout(fanout),
			nvm_level(nvm_level) {}

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

  // Search nearest neighbor
	cs_node_t* search_nearest(tuple_t& target);
	
	// Print the tree
	void display(); 

private:
	// Dimension of points stored in the tree
	int dimension; 	

	// Fanout of tree, default is 2
	int fanout

	// How many levels from leaf is stored in nvm,
	// nvm_level=0 has entrie tree in memory
	int nvm_level;

	// Helper function for buildfrom
	cs_node_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, cs_node_t* parent, int& height);
	
	// Find parent node of tuple if it were to be inserted into tree rooted at starter
	cs_node_t* find_parent(cs_node_t* starter, tuple_t& tuple);
	
	void display_helper(cs_node_t* node, string label);
 	
	cs_node_t* search_nearest_helper(cs_node_t* starter, tuple_t& target);
};
