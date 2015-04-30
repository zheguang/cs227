// @xl242
#include <cstdlib>
#include <iostream>
#include "kd-tree.hpp"
#include "../FatalError.hpp"

//===========================================================
//    KDTREE PUBLIC FUNCTIONS
//===========================================================

/* A deconstructor to free all memory allocated */
tree_t::~tree_t() {
	if (root == NULL) return;
	free_tree_helper(root);
}

void tree_t::buildfrom(vector<tuple_t>& points) {
	check_config(points.size());
	root = buildfrom_helper(points, 0, points.size()-1, 0, NULL);
}

node_t* tree_t::insert(tuple_t& tuple, HybridMemory::MEMORY_NODE_TYPE type) {
	node_t* newnode = (node_t*)HybridMemory::alloc(sizeof(node_t), type);
	assert(config.fanout == 2);
	newnode->value = tuple;
	newnode->left = NULL;
	newnode->right = NULL;
	newnode->parent = NULL;
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
	if (willbe_child == 0) {
		parent->left = newnode;
	} else {
		parent->right = newnode;
	}
	return newnode;
}
	
void tree_t::remove(node_t* node) {
	assert(config.fanout == 2);
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
}


void tree_t::display() const {
	cout << "kd-tree: \n";
	display_helper(root, "");
}

node_t* tree_t::search_nearest(tuple_t& target) const {
	return search_nearest_helper(root, target);
}

//===========================================================
//      KDTREE PRIVATE FUNCTIONS
//===========================================================

void tree_t::free_node(node_t* node) {
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
}

void tree_t::free_tree_helper(node_t* start) {
	if (start->left != NULL) free_tree_helper(start->left);
	if (start->right != NULL) free_tree_helper(start->right);
	free_node(start);
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

// Base on pesudo-code on wikipedia
node_t* tree_t::buildfrom_helper(
		vector<tuple_t>& points,
		int lbd, int rbd, int depth, node_t* parent) const {
	assert(config.fanout == 2);
	if (lbd > rbd) return NULL;
	int median_idx = (rbd + lbd) / 2;
	if (!KD_KEY_SORTED) {
		int axis = depth % config.dimension;
		int right_median = (rbd - lbd + 1) / 2;
		if ((rbd - lbd + 1) % 2 == 0) right_median--;
		median_idx = quickfind_tuples_by_axis(
				points, lbd, rbd, axis, right_median);
	}

	// Determine where node resides base on node height from leaves
	node_t* newnode = NULL;
	// Reminder, size of node depends on dimension as well
	int nodesize = sizeof(node_t) + sizeof(datatype_t) * config.dimension;
	int height = bottomheight(rbd - lbd + 1, config.fanout);
	if (shouldbe_inmemory(height, depth)) {
		newnode = (node_t*)HybridMemory::alloc(
				nodesize, HybridMemory::DRAM);
	} else {
		newnode = (node_t*)HybridMemory::alloc(
				nodesize, HybridMemory::NVM);
	}
	newnode->parent = parent;
	newnode->depth = depth;
	// XXX Is thi copy memory data to NVM ??
	newnode->value = points[median_idx];

	newnode->left = buildfrom_helper(
			points, lbd, median_idx-1, depth+1, newnode);
	newnode->right = buildfrom_helper(
			points, median_idx+1, rbd, depth+1, newnode);
	return newnode;
}

void tree_t::display_helper(node_t* node, string label) const {
	assert(config.fanout == 2);
	if (node == NULL) return;
	cout << string(2*node->depth, ' ') << label;
	cout << tuple_string(node->value);
	// Debug statement
	if (KD_DEBUG) {
		cout <<" :" << node;
		cout << " -> " << node->parent;
	}
	cout << "\n";
	display_helper(node->left, "L: ");
	display_helper(node->right, "R: ");
}

node_t* tree_t::find_parent(
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
}

node_t* tree_t::find_replacement(node_t* replaced) const {
	assert(config.fanout == 2);
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
	assert(config.fanout == 2);
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
	assert(config.fanout == 2);
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
}

node_t* tree_t::search_nearest_helper(
		node_t* starter, tuple_t& target) const {
	assert(config.fanout == 2);
	if (starter == NULL) return NULL;
	int willbe_child = 0;
	node_t* cur_best = find_parent(starter, target, willbe_child);
	if (willbe_child == -1) {
		return cur_best;
	}
	
	datatype_t cur_dist = distance(cur_best->value, target);
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
}

