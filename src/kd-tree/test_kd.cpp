// @xl242
#include <ctime>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "kd-tree.hpp"

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


void experiment_randomnns(
		tree_t* kdtree, vector<tuple_t>& points,
		int num_trials, int randbase) {
	node_t* nearest = NULL;
	for (int i = 0; i < num_trials; i++) {
		tuple_t target = generate_tuple(kdtree->get_dimension(), randbase);		
		if (KD_DEBUG) {
			cout << tuple_string(target) << " " << i << "\n";
		}
		nearest = kdtree->search_nearest(target);
		assert(is_nearest(points, target, nearest->value));
	}
}

void experiment_inputnns(
		tree_t* kdtree, vector<tuple_t>& points) {
	node_t* nearest = NULL;
	for (unsigned int i = 0; i < points.size(); i++) {
		tuple_t target = points[i];
		nearest = kdtree->search_nearest(target);
		assert(nearest != NULL);
	}
}

void testInsertRemove() {
	cout << "Lets test kd's insert and remove\n";
	int num_points = 10;
	int dimension = 5;
	int base = 1000;
	int fanout = 2;
	config_t config(
			dimension, 
			BY_PERCENTILE,
			1.0,
			fanout);
	tree_t kdtree(config);
	vector<tuple_t> points;
	vector<node_t*> nodes;
	for (int i = 0; i < num_points; i++) {
		tuple_t tuple = generate_tuple(dimension, base);
		points.push_back(tuple);
		nodes.push_back(kdtree.insert(tuple, hmindex::HybridMemory::DRAM));
	}
	int num_trials = 100;
	experiment_randomnns(&kdtree, points, num_trials, base); 

	for (int i = 0; i < num_points; i++) {
		remove_point_fr_pool(points, kdtree.root->value);
		kdtree.remove(kdtree.root);
		if (kdtree.root != NULL) {
			experiment_randomnns(&kdtree, points, num_trials, 2*base); 
		}
	}
	assert(kdtree.root == NULL);
}


void testSingleDimension(string pathname) {
	cout << "Test for single dimension\n";
	int dimension = 1;
	int fanout = 2; // fanout must be 2
	int num_trials = 2000;
	int base = 200000;
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.0, // Let half of the tree in memory
			fanout);
	tuple_t target(dimension);
	
	tree_t bst(config);
	time_t tstart = time(0);
	bst.buildfrom(points);
	time_t tend = time(0);
	cout << "finished building the tree. Used ";
	cout << difftime(tend, tstart) << "s\n";
	if (KD_DEBUG) {
		bst.display();
	}
	// Test knn search
	cout << "start nns...\n";
	tstart = time(0);
	experiment_randomnns(&bst, points, num_trials, base);
	tend = time(0);
	cout << "Used: "<< difftime(tend, tstart) << "s\n";
}


void testMultipleDimension(string pathname) {
	cout << "Lets test multiple dimension kd\n";
	int dimension = 3;
	int fanout = 2; // XXX fanout must be 2 at this point!!!
	int num_trials = 20;
	int base = 200;
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.99, // Let half of the tree in memory
			fanout);
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	tuple_t target;
	target.resize(dimension);

	tree_t kdtree(config);
	kdtree.buildfrom(points);
	if (KD_DEBUG) {
		kdtree.display();
	}
	experiment_randomnns(&kdtree, points, num_trials, base);
}

void testSpeed(string pathname, int dimension) {
	cout << "Test for speed\n";
	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	config_t config( dimension, BY_PERCENTILE, 0.0, 2);
	tuple_t target(dimension);
	
	tree_t tree(config);
	time_t tstart = time(0);
	tree.buildfrom(points);
	time_t tend = time(0);
	cout << "finished building the tree. Used ";
	cout << difftime(tend, tstart) << "s\n";
	
	// Test knn search
	cout << "start nns...\n";
	tstart = time(0);
	experiment_inputnns(&tree, points);
	tend = time(0);
	cout << "Used: "<< difftime(tend, tstart) << "s\n";
}

int main(int argc, char** argv) {
	if (argc != 3) {
		cout << "Please define a data source and dimension\n";
		exit(1);
	}
	string filename(argv[1]);
	cout << "build tree from " << filename << "\n";
	int dimension = atoi(argv[2]);
	testSpeed(filename, dimension);


//	testInsertRemove();
//	testSingleDimension(filename);
//	testMultipleDimension(filename);
	return 0;
}
