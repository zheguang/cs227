#include <cstdlib>
#include <iostream>
#include "kd-tree.hpp"
#include "../HybridMemory.hpp"
using namespace hmindex;

/* Methods for kd-tree */
void tree_t::check_config(int num_points) {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			nvm_level = config.value;
			break;
		case BY_PERCENTILE:
			memory_depth = int(float(bottomheight(num_points)) *
									 (1.0 - config.value));
			break;
	} // Switch
}

bool tree_t::inMemory(int h, int d) const {
	switch (config.policy) {
		case BY_HEIGHT_FROM_BOTTOM:
			return h > nvm_level;
		case BY_PERCENTILE:
			return d <= memory_depth; 
		default:
			return false;
	}
}

void tree_t::buildfrom(vector<tuple_t>& points) {
	check_config(points.size());
	root = buildfrom_helper(points, 0, points.size()-1, 0, NULL);
}

// Base on pesudo-code on wikipedia
node_t* tree_t::buildfrom_helper(
		vector<tuple_t>& points,
		int lbd, int rbd, int depth, node_t* parent) const {
	if (lbd > rbd) return NULL;
	int axis = depth % config.dimension;
	int right_median = (rbd - lbd + 1) / 2;
	if ((rbd - lbd + 1) % 2 == 0) right_median--;
	int median_idx = quickfind_tuples_by_axis(
			points, lbd, rbd, axis, right_median);

	// Determine where node resides base on node height from leaves
	node_t* newnode = NULL;
	int height = bottomheight(rbd - lbd + 1);
	if (inMemory(height, depth)) {
		newnode = (node_t*)HybridMemory::alloc(
				sizeof(node_t), HybridMemory::DRAM);
	} else {
		newnode = (node_t*)HybridMemory::alloc(
				sizeof(node_t), HybridMemory::NVM);
	}
	newnode->parent = parent;
	newnode->depth = depth;
	newnode->value = points[median_idx];

	newnode->left = buildfrom_helper(
			points, lbd, median_idx-1, depth+1, newnode);
	newnode->right = buildfrom_helper(
			points, median_idx+1, rbd, depth+1, newnode);
	return newnode;
}

void tree_t::display() const {
	cout << "kd-tree: \n";
	display_helper(root, "");
}

void tree_t::display_helper(node_t* node, string label) const {
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

node_t* tree_t::find_parent(node_t* starter, tuple_t& target) const {
	if (starter == NULL) return NULL;
	node_t* key = starter;
	while (true) {
		if (key->value == target) return key;
		int axis = key->depth % config.dimension;
		if (target[axis] < key->value[axis]) {
			if (key->left == NULL) return key;
			key = key->left;
		} else {
			if (key->right == NULL) return key;
			key = key->right;
		}
	}
	return NULL;
}

node_t* tree_t::search_nearest(tuple_t& target) const {
	return search_nearest_helper(root, target);
}

node_t* tree_t::search_nearest_helper(
		node_t* starter, tuple_t& target) const {
	if (starter == NULL) return NULL;
	node_t* cur_best = find_parent(starter, target);
	float cur_dist = distance(cur_best->value, target);
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