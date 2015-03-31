#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <unordered_set>
using std::cout;
using std::string;
using std::vector;
using std::ostringstream;

typedef vector<float> tuple_t;

// Print a tuple_t
inline string tuple_string(tuple_t t) {
	ostringstream buf;
	buf << "(";
	for (unsigned int i = 0; i < t.size(); i++) {
		buf << t[i] << " ";
	} buf << ")";
	return buf.str();
}

// calculate norm2 distance between two points
inline float distance(tuple_t& t1, tuple_t& t2) {
	if (t1.size() != t2.size()) return -float(1);
	float ret = 0.0;
	for (unsigned int i = 0; i < t1.size(); i++) {
		ret += (t1[i] - t2[i]) * (t1[i] - t2[i]);
	}
	return pow(ret, 0.5);
}

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
	tree_t(int d, int nvm_level=0) : 
			root(NULL), 
			dimension(d),
			nvm_level(nvm_level) {}

	// Build a tree from a list of points, 
	void buildfrom(vector<tuple_t>& points);

	// Insert new node into tree
	void insert(node_t newnode);

  // Search nearest neighbor
	node_t* search_nearest(tuple_t& target);
	
	// Print the kd-tree
	void display(); 

private:
	// Dimension of points stored in the tree
	int dimension; 	

	// How many levels from leaf is stored in nvm,
	// nvm_level=0 has entrie tree in memory
	int nvm_level;

	// Helper function for buildfrom
	node_t* buildfrom_helper(
			vector<tuple_t>& points,
			int lbd, int rbd, int depth, node_t* parent, int& height);
	
	// Find parent node of tuple if it were to be inserted into tree rooted at starter
	node_t* find_parent(node_t* starter, tuple_t& tuple);
	
	void display_helper(node_t* node, string label);
 	
	node_t* search_nearest_helper(node_t* starter, tuple_t& target);
};
