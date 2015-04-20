// @xl242
#include <ctime>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include "kd-tree.hpp"

/* A helper function for testing */
tuple_t generate_tuple(int dimension, int base) {
	tuple_t t(dimension);
	for (int i = 0; i < dimension; i++) {
		int b = rand() % base + 10;
		t[i] = double(rand() % b);
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
	double shortest_d = distance(target, nearest);
	for (unsigned int i = 0; i < points.size(); i++) {
		double d = distance(points[i], target);
		if (d < shortest_d) {
			if (KD_DEBUG) {
				cout << "err nns for " << tuple_string(target);
				cout << "Got " << tuple_string(nearest) << " (" << shortest_d << "), ";
				cout << "Found " << tuple_string(points[i]) << " (" << d << ").\n";
			}
			return false;
		}
	}
	return true;
}

void testSingleDimension(string pathname) {
	cout << "Test for single dimension\n";
	int dimension = 1;
	int fanout = 2; // XXX fanout must be 2 at this point!!!
	int num_trials = 1;
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.0, // Let half of the tree in memory
			fanout);
	tuple_t target(dimension);
	
	double percentile = 0.95;
	for (int k = 0; k < 1; k++) {
		config.value = percentile;
		tree_t bst(config);
		time_t tstart = time(0);
		bst.buildfrom(points);
		time_t tend = time(0);
		cout << "finished building the tree. Used " << difftime(tend, tstart) << "s\n";
		if (KD_DEBUG) {
			bst.display();
		}
		// Test knn search
		node_t* nearest = NULL;
		cout << "start nns...\n";
		tstart = time(0);
		for (int i = 0; i < num_trials; i++) {
			// target = generate_tuple(dimension, 1);
			target[0] = 1.0;
			nearest = bst.search_nearest(target);
			assert(is_nearest(points, target, nearest->value));
		}
		tend = time(0);
		cout << "finished nns of " << percentile << " ";
		cout << "Used: "<< difftime(tend, tstart) << "s\n";
		percentile -= 0.5;
	}
}


void testMultipleDimension() {
	cout << "Lets test multiple dimension kd\n";
	int dimension = 1;
	int fanout = 2; // XXX fanout must be 2 at this point!!!
	int num_trials = 20;
	int num_points = 100;
	int base = 200;
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.5, // Let half of the tree in memory
			fanout);
	tuple_t target;
	node_t* nearest = NULL;

	for (dimension = 2; dimension < 10; dimension++) {
		target.resize(dimension);
		config.dimension = dimension;
		config.value = double(rand() % 10) / 10;
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
		}
	}
}


int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "Please define a data source\n";
		exit(1);
	}
	string filename(argv[1]);
	cout << "build tree from " << filename << "\n";
	testSingleDimension(filename);
	return 0;
}
