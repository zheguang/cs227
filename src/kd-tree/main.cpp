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
			cout << "err found for " << tuple_string(target) << "\n";
		 	cout	<< "found " << tuple_string(nearest) << " with distance " << shortest_d << "\n";
			cout << "got nearer one " << tuple_string(points[i]) << " of d = " << d << "\n";
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
	tuple_t target(dimension);
	node_t* nearest = NULL;

	tree_t bst(dimension, 1); // Just test a normal BST
	vector<tuple_t> numbers = generate_tuples(dimension, num_points); 
	bst.buildfrom(numbers);
	bst.display();
	// Test search
	for (int i = 0; i < num_trials; i++) {
		target = generate_tuple(dimension, base);
		nearest = bst.search_nearest(target);
		assert(is_nearest(numbers, target, nearest->value));
	}

	for (dimension = 2; dimension < 10; dimension++) { // Test on points on R^n
		target.resize(dimension);
		tree_t kdtree(dimension);
		vector<tuple_t> points = generate_tuples(dimension, num_points); 
		kdtree.buildfrom(points);
		kdtree.display();
		for (int i = 0; i < num_trials; i++) {
			target = generate_tuple(dimension, base);		
			cout << tuple_string(target) << " " << i << "\n";
			nearest = kdtree.search_nearest(target);
			assert(is_nearest(points, target, nearest->value));
		}/**/
	}
	return 0;
}
