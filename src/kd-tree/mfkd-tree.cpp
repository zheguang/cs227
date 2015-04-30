// @xl242
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <cstdlib>
#include <limits.h>
#include <iostream>
#include <string.h>
#include <string>
#include "mfkd-tree.hpp"
#include "../FatalError.hpp"

//===========================================================
//    MFKDTREE PUBLIC FUNCTIONS
//===========================================================

/* A deconstructor to free all memory allocated */
tree_t::~tree_t() {
	if (root == NULL) return;
	free_tree_helper(root);
}

void tree_t::buildfrom(vector<tuple_t>& points) {
	check_config(points.size());
	root = buildfrom_helper(points, 0, points.size()-1, 0, NULL, 0);
}

/*node_t* tree_t::mf_insert(
		tuple_t& tuple,
		HybridMemory::MEMORY_NODE_TYPE stype
		HybridMemory::MEMORY_NODE_TYPE ctype) {
	node_t* newnode = (node_t*)HybridMemory::alloc(sizeof(node_t), stype);
	newnode->value = tuple;
	newnode->parent = NULL;
	newnode->children = HybridMemory::alloc(config.fanout * sizeof(node_t), ctype);
	if (root == NULL) {
		root = newnode;
		return newnode;
	}
	int willbe_child = -1;
	node_t* parent = find_parent(root, tuple, willbe_child);
	if (parent == NULL) {
		cout << "bad\n";
	}
	newnode->parent = parent;
	newnode->depth = parent->depth + 1;
	insert_childnode(parent, childindex, newnode);
	return newnode;
}*/
	
/*void tree_t::remove(node_t* node) {
	if (node == NULL) return;
	if (node->left == NULL && node->right == NULL) {
		if (node == root) {
			root = NULL;
			free_node(node);
			return;
		}
		if (node == node->parent->left) {
			node->parent->left = NULL;
		} else {
			node->parent->right = NULL;
		}
		free_node(node);
		return;
	}
	node_t* replacement = find_replacement(node);
	node->value = replacement->value;
	remove(replacement);
}*/


void tree_t::display() const {
	cout << "kd-tree: \n";
	display_helper(root, "");
}

/*node_t* tree_t::search_nearest(tuple_t& target) const {
	return search_nearest_helper(root, target);
}*/

//===========================================================
//      KDTREE PRIVATE FUNCTIONS
//===========================================================

/*void tree_t::free_node(node_t* node) {
	void* children = node->children;
	// Free node itself
	try{ 
		HybridMemory::assertAddress(node, HybridMemory::DRAM);
		HybridMemory::free(node, sizeof(node_t), HybridMemory::DRAM);
	} catch (FatalError& err) {
		try {
			HybridMemory::assertAddress(node, HybridMemory::NVM);
			HybridMemory::free(node, sizeof(node_t), HybridMemory::NVM);
		} catch (FatalError& err) {
			cout << "Encounter invalid memory type " << node << "\n";
		}
	}

	// Free children
	// TODO
}*/

void tree_t::free_tree_helper(node_t* start) {
//	if (start->left != NULL) free_tree_helper(start->left);
//	if (start->right != NULL) free_tree_helper(start->right);
//	free_node(start);
}

void tree_t::check_config(int num_points) {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			nvm_level = config.value;
			break;
		case BY_PERCENTILE:
			cout << num_points << " points in total\n";
			int num_inmemory_nodes = int(num_points * (1.0 - config.value));
			int memory_depth = 0;
			if (num_inmemory_nodes != 0) {
				memory_depth = bottomheight(num_inmemory_nodes, config.fanout);
			}
			cout << memory_depth << " depth of nodes will be in memory\n";
			break;
	} // Switch
}

bool tree_t::shouldbe_inmemory(int h, int d) const {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			return h > nvm_level;
		case BY_PERCENTILE:
			return d <= memory_depth; 
		default:
			return false;
	}
}

node_t* tree_t::get_child(node_t* parent, int index) const {
	if (index >= config.fanout) {
		throw "bad";
	}
	int nodesize = sizeof(node_t) +
			sizeof(datatype_t) * config.dimension * (config.fanout - 1);
	char* ptr = (char*)parent->children;
	for (int i = 0; i < index; i++) {
		for (int k = 0; k < nodesize; k++) ptr++;
	}
	return (node_t*)ptr;
}

// Base on pesudo-code on wikipedia
node_t* tree_t::buildfrom_helper(
		vector<tuple_t>& points,
		int lbd, int rbd, int depth, node_t* parent, int childindex) const {
	if (lbd > rbd) return NULL;
	
	// Assume key all sorted at this point
	assert(MFKD_KEY_SORTED);
	vector<int> lbds;
	if (lbd != rbd) {
		int unitsize = (rbd - lbd + 1) / config.fanout;
		int index = 0;
		for (int i = 0; i < config.fanout; i++) {
			lbds.push_back(lbd + index);
			index += std::max(1, unitsize);
			if (lbd + index > rbd) break;
		}

		if (MFKD_DEBUG) {
			cout << "lbds: \n";
			for (unsigned int i = 0; i < lbds.size(); i++) {
				cout << lbds[i] << " ";
			} cout << "\n";
		}
	}
	node_t* newnode = NULL;
	int nodesize = sizeof(node_t) +
			sizeof(datatype_t) * config.dimension * (config.fanout - 1);
	int height = bottomheight(rbd - lbd + 1, config.fanout);
	if (parent != NULL) {
		if (parent->num_children == 0) {
			if (shouldbe_inmemory(depth, height)) {
				parent->children = HybridMemory::alloc(
						nodesize * config.fanout, HybridMemory::DRAM);
			} else {
				parent->children = HybridMemory::alloc(
						nodesize * config.fanout, HybridMemory::NVM);
			}
			memset(parent->children, 0, nodesize * config.fanout);
		}
		newnode = get_child(parent, childindex);
		parent->num_children++;
	} else {
		if (shouldbe_inmemory(depth, height)) {
			newnode = (node_t*)HybridMemory::alloc(
					sizeof(node_t), HybridMemory::DRAM);
		} else {
			newnode = (node_t*)HybridMemory::alloc(
					sizeof(node_t), HybridMemory::NVM);
		}
	}
	newnode->depth = depth;
	newnode->parent = parent;
	newnode->num_children = 0;
	newnode->childindex = childindex;
	if (rbd - lbd + 1 <= config.fanout - 1) {
		for (int i = lbd; i <= rbd; i++) {
			newnode->values.push_back(points[i]);
		}
		return newnode;
	}

	for (unsigned int i = 1; i < lbds.size(); i++) {
		newnode->values.push_back(points[lbds[i]]);
	}
	buildfrom_helper(points, lbds[0], lbds[1]-1, depth+1, newnode, 0);
	for (unsigned int i = 1; i < lbds.size() - 1; i++) {
		buildfrom_helper(points, lbds[i]+1, lbds[i+1]-1, depth+1, newnode, i);
	}
	buildfrom_helper(points, lbds.back()+1, rbd, depth+1, newnode, lbds.size()-1);
	return newnode;
}

bool tree_t::is_null(node_t* node) const {
	int nodesize = sizeof(node_t) +
			sizeof(datatype_t) * config.dimension * (config.fanout - 1);
	char* nullchild = (char*)malloc(nodesize);
	memset(nullchild, 0, nodesize);
	int ret = strncmp((char*)node, nullchild, nodesize);
	free(nullchild);
	return ret == 0;
}

void tree_t::display_helper(node_t* node, string label) const {
	if (node == NULL) return;
	cout << string(2*node->depth, ' ') << label;
	for (unsigned int i = 0; i < node->values.size() - 1; i++) {
		cout << tuple_string(node->values[i]) << "|";
	} cout << tuple_string(node->values.back());

	// Debug statement
	if (MFKD_DEBUG) {
		cout <<" :" << node;
		cout << " -> " << node->parent;
	}
	cout << "\n";
	if (node->num_children == 0) return;
	for (int i = 0; i < config.fanout; i++) {
		node_t* child = get_child(node, i);
		bool isnull = is_null(child);
		if (!isnull) {
			display_helper(child, std::to_string(i) + ": ");
		}
	}
}

node_t* tree_t::find_parent(
		node_t* starter, tuple_t& target, int& willbe_child) const {
	if (starter == NULL) return NULL;
	node_t* key = starter;
	while (true) {
	search:
		for (unsigned int i = 0; i < key->values.size(); i++) {
			if (key->values[i] == target) {
				willbe_child = -1;
				return key;
			}
		}
		int axis = key->depth % config.dimension;
		if (key->num_children == 0) {
			willbe_child = 0;
			while(willbe_child < (int)key->values.size() && 
					target[axis] >= key->values[willbe_child][axis]) {
				willbe_child++;
			}
			return key;	
		}
		for (unsigned int i = 0; i < key->values.size(); i++) {
			if (target[axis] < key->values[i][axis]) {
				node_t* child = get_child(key, i);
				if (is_null(child)) {
					willbe_child = i;
					return key;
				}
				key = child;
				goto search;
			}
		}
		node_t* lastchild = get_child(key, key->values.size());
		if (is_null(lastchild)) {
			willbe_child = (int)key->values.size();
			return key;
		}
		key = lastchild;
	}
	return NULL;
}

/*node_t* tree_t::find_replacement(node_t* replaced) const {
	if (replaced == NULL) return NULL;
	int axis = replaced->depth % config.dimension;
	if (replaced->right != NULL) {
		return find_smallest(replaced->right, axis);
	}
	if (replaced->left != NULL) {
		return find_largest(replaced->left, axis);
	}
	// A leaf node
	return replaced;
}

node_t* tree_t::find_largest(node_t* start, int comp_axis) const {
	if (start == NULL) return start;
	int axis = start->depth % config.dimension;
	if (axis == comp_axis) {
		if (start->right == NULL) return start;
		return find_largest(start->right, comp_axis);
	}
	node_t* lc = find_largest(start->left, comp_axis);
	node_t* rc = find_largest(start->right, comp_axis);
	node_t* replacement = start;
	if (lc != NULL && replacement->value[comp_axis] < lc->value[comp_axis]) {
		replacement = lc;
	}
	if (rc != NULL && replacement->value[comp_axis] < rc->value[comp_axis]) {
		replacement = rc;
	}
	return replacement;
}

node_t* tree_t::find_smallest(node_t* start, int comp_axis) const {
	if (start == NULL) return start;
	int axis = start->depth % config.dimension;
	if (axis == comp_axis) {
		if (start->left == NULL) return start;
		return find_smallest(start->left, comp_axis);
	}
	node_t* lc = find_smallest(start->left, comp_axis);
	node_t* rc = find_smallest(start->right, comp_axis);
	node_t* replacement = start;
	if (lc != NULL && replacement->value[comp_axis] > lc->value[comp_axis]) {
		replacement = lc;
	}
	if (rc != NULL && replacement->value[comp_axis] > rc->value[comp_axis]) {
		replacement = rc;
	}
	return replacement;
}*/

datatype_t smallest_distdiff_innode(
		node_t* node, int selectindex, tuple_t* target) {
	datatype_t cur_dist = (datatype_t)LLONG_MAX;
	if (selectindex < node->values.size()) {
		cur_dist = min(cur_dist, distance(node->values[selectindex], target));
	}
	if (selectindex > 0) {
		cur_dist = min(cur_dist, distance(node->values[selectindex-1], target));
	}
	return cur_dist;
}

node_t* tree_t::search_nearest_helper(
		node_t* starter, tuple_t& target) const {
	if (starter == NULL) return NULL;
	int willbe_child = 0;
	node_t* cur_best = find_parent(starter, target, willbe_child);
	if (willbe_child == -1) {
		return cur_best;
	}

	datatype_t cur_dist = smallest_distdiff_inode(cur_best, willbe_child, target);
	if (cur_best->num_children != 0 ) {
		for (int i = 0; i < config.fanout; i++) {
			node_t* child = get_child(cur_best, i);
			bool isnull = is_null(child);
			if (isnull) continue;
			node_t* candidate = search_nearest_helper(child, target);
				datatype_t canddist = (datatype_t)LLONG_MAX;
				for (unsigned int i = 0; i < candidate->values.size(); i++) {
					canddist = min(canddist, distance(candidate->values[i], target);
				}
				if (canddist < cur_dist) {
					cur_best = candidate;
					cur_dist = canddist;
				}
			}
		}
	}

	node_t* key = cur_best->parent;
	int childindex = cur_best->childindex;
	while (key != NULL && prev != starter) {
		datatype_t d = smallest_distdiff_innode(key, curbest->childindex, target);
		if (d < cur_dist) {
			cur_best = key;
			cur_dist = d;
		}
		int axis = key->depth % config.dimension;
		if (abs(target[axis] - key->values[childindex][axis]) < cur_dist) {


	}


	node_t* prev = cur_best;
	while (key != NULL && prev != starter) {
		// Check value on node
		if (distance(key->value, target) < cur_dist) {
			cur_best = key;
			cur_dist = distance(key->value, target);
		}
		int axis = key->depth % config.dimension;
		if (abs(target[axis] - key->value[axis]) < cur_dist) {
			// Need to go to the other branch
			node_t* candidate = NULL;
			if (prev == key->left) { // We were previously in left branch
				candidate = search_nearest_helper(key->right, target);
			} else if (prev == key->right) { // Previously on right
				candidate = search_nearest_helper(key->left, target);
			}
			// Compare with cur_dist
			if (candidate != NULL && distance(candidate->value, target) < cur_dist) {
				cur_best = candidate;
				cur_dist = distance(candidate->value, target);
			}
		}
		prev = key;
		key = key->parent;
	}
	return cur_best;
}

