// @xl242
#include <assert.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
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
	node_t* ptr = (node_t*)parent->children;
	for (int i = 0; i < index; i++) ptr++;
	return ptr;
}

// Base on pesudo-code on wikipedia
node_t* tree_t::buildfrom_helper(
		vector<tuple_t>& points,
		int lbd, int rbd, int depth, node_t* parent, int childindex) const {
	if (lbd > rbd) return NULL;
	
	// Assume key all sorted at this point
	assert(MFKD_KEY_SORTED);
	int median_index = (rbd + lbd) / 2;
	vector<int> lbds;
	if (lbd != rbd) {
		int unitsize = (rbd - lbd + 1) / config.fanout;
		int index = 0;
		for (int i = 0; i < config.fanout; i++) {
			lbds.push_back(lbd + index);
			index += std::max(1, unitsize);
			if (index > rbd) break;
		}

		if (MFKD_DEBUG) {
			cout << "lbds: \n";
			for (unsigned int i = 0; i < lbds.size(); i++) {
				cout << lbds[i] << " ";
			} cout << "\n";
		}
	}
	node_t* newnode = NULL;
	int height = bottomheight(rbd - lbd + 1, config.fanout);
	if (parent != NULL) {
		if (parent->num_children == 0) {
			if (shouldbe_inmemory(depth, height)) {
				parent->children = HybridMemory::alloc(
						sizeof(node_t) * config.fanout, HybridMemory::DRAM);
			} else {
				parent->children = HybridMemory::alloc(
						sizeof(node_t) * config.fanout, HybridMemory::NVM);
			}
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
	newnode->value = points[median_index];
	newnode->depth = depth;
	newnode->parent = parent;
	newnode->num_children = 0;

	for (unsigned int i = 0; i < lbds.size(); i++) {
		if (i < lbds.size() - 1) {
			buildfrom_helper(points, lbds[i], lbds[i+1]-1, depth+1, newnode, i);
		} else {
			buildfrom_helper(points, lbds[i], rbd, depth+1, newnode, i);
		}
	}
	return newnode;
}

void tree_t::display_helper(node_t* node, string label) const {
	if (node == NULL) return;
	cout << string(2*node->depth, ' ') << label;
	cout << tuple_string(node->value);
	// Debug statement
	if (MFKD_DEBUG) {
		cout <<" :" << node;
		cout << " -> " << node->parent;
	}
	cout << "\n";
	for (int i = 0; i < node->num_children; i++) {
		node_t* child = get_child(node, i);
		display_helper(child, std::to_string(i)+" :");
	}
}

/*node_t* tree_t::find_parent(
		node_t* starter, tuple_t& target, int& willbe_child) const {
	if (starter == NULL) return NULL;
	node_t* key = starter;
	while (true) {
		if (key->value == target) {
			willbe_child = -1;
			return key;
		}
		int axis = key->depth % config.dimension;
		// Canonical
		if (target[axis] < key->value[axis]) {
			if (key->left == NULL) {
				willbe_child = 0;
				return key;
			}
			key = key->left;
		} else {
			if (key->right == NULL) {
				willbe_child = 1;
				return key;
			}
			key = key->right;
		}
	}
	return NULL;
}*/

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

/*node_t* tree_t::search_nearest_helper(
		node_t* starter, tuple_t& target) const {
	if (starter == NULL) return NULL;
	int willbe_child = -1;
	node_t* cur_best = find_parent(starter, target, willbe_child);
	assert(willbe_child == -1);
	double cur_dist = distance(cur_best->value, target);
	node_t* left_branch = cur_best->left != NULL ? cur_best->left:
																								 cur_best->right;
	if (left_branch != NULL) {
		// Make sure we start query from a leaf node
		node_t* candidate = search_nearest_helper(left_branch, target);
		if (distance(candidate->value, target) < cur_dist) {
			cur_best = candidate;
			cur_dist = distance(candidate->value, target);
		}
	}

	node_t* key = cur_best->parent;
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
}*/

