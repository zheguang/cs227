// @xl242
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include "kd-tree.hpp"

/* A helper function for testing */
tuple_t generate_tuple(int dimension, int base) {
	tuple_t t(dimension);
	for (int i = 0; i < dimension; i++) {
		int b = rand() % base + 10;
		t[i] = float(rand() % b);
	}
	return t;
}

vector<tuple_t> generate_tuples(int dimension, int size) {
	vector<tuple_t> ret;
	for (int i = 0; i < size; i++) {
		ret.push_back(generate_tuple(dimension, 200));
	}
	return ret;
}

bool is_nearest(vector<tuple_t>& points, tuple_t& target, tuple_t& nearest) {
	float shortest_d = distance(target, nearest);
	for (unsigned int i = 0; i < points.size(); i++) {
		float d = distance(points[i], target);
		if (d < shortest_d) {
			if (KD_DEBUG) {
				cout << "err found for " << tuple_string(target) << "\n";
				cout << "found " << tuple_string(nearest);
				cout << " with distance " << shortest_d << "\n";
				cout << "got nearer one " << tuple_string(points[i]);
				cout << " of d = " << d;
				cout << "\n";
			}
			return false;
		}
	}
	return true;
}

int main() {
	int dimension = 1;
	int num_points = 50;
	int num_trials = 20;
	int base = 200;
	config_t config(
			dimension, 
			BY_HEIGHT_FROM_BOTTOM,
			bottomheight(num_points)/2);
	tuple_t target(dimension);
	node_t* nearest = NULL;

	// Just test a normal BST
	printf("Test for single dimension\n");
	tree_t bst(config);
	vector<tuple_t> numbers = generate_tuples(dimension, num_points); 
	bst.buildfrom(numbers);
	if (KD_DEBUG) {
		bst.display();
	}
	// Test knn search
	for (int i = 0; i < num_trials; i++) {
		target = generate_tuple(dimension, base);
		nearest = bst.search_nearest(target);
		assert(is_nearest(numbers, target, nearest->value));
	}

	printf("Test for multi dimension\n");
	config.policy = BY_PERCENTILE;
	// Test on points on R^n
	for (dimension = 2; dimension < 10; dimension++) {
		target.resize(dimension);
		config.dimension = dimension;
		config.value = float(rand() % 10) / 10;
		tree_t kdtree(config);
		vector<tuple_t> points = generate_tuples(dimension, num_points); 
		kdtree.buildfrom(points);
		if (KD_DEBUG) {
			kdtree.display();
		}
		for (int i = 0; i < num_trials; i++) {
			target = generate_tuple(dimension, base);		
			if (KD_DEBUG) {
				cout << tuple_string(target) << " " << i << "\n";
			}
			nearest = kdtree.search_nearest(target);
			assert(is_nearest(points, target, nearest->value));
		}/**/
	}
	return 0;
}
