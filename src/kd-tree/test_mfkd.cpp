// @xl242
#include <ctime>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <stack>
#include <unordered_map>
#include "mfkd-tree.hpp"

bool is_nearest(vector<tuple_t>& points, tuple_t& target, datatype_t actual_sdist) {
	for (unsigned int i = 0; i < points.size(); i++) {
		double d = distance(points[i], target);
		if (d < actual_sdist) {
			if (MFKD_DEBUG) {
				cout << "err nns for " << tuple_string(target);
				cout << "Found nearer: " << tuple_string(points[i]) << " (" << d << ").\n";
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
		datatype_t sdist;
		nearest = kdtree->search_nearest(target, sdist);
		if (MFKD_DEBUG) {
			cout << "found nearest for " << tuple_string(target);
			cout << ": " << sdist << " as ";
			kdtree->print_node(nearest);
		}
		assert(is_nearest(points, target, sdist));
	}
}

/*
void testInsertRemove() {
	cout << "Lets test kd's insert and remove\n";
	int num_points = 200;
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
}*/


void testSingleDimension(string pathname) {
	cout << "Test for single dimension\n";
	int dimension = 1;
	int fanout = 3;
	int num_trials = 1;
	int base = 200;
//	vector<tuple_t> points = createTuplesFromFile(pathname, dimension); 
	vector<tuple_t> points = generate_sortedtuples(dimension, 20);
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.0,
			fanout);
	tuple_t target(dimension);
	
	tree_t tree(config);
	time_t tstart = time(0);
	tree.buildfrom(points);
	time_t tend = time(0);
	cout << "finished building the tree. Used ";
	cout << difftime(tend, tstart) << "s\n";
	if (MFKD_DEBUG) {
		tree.display();
	}

	// Test knn search
	cout << "start nns...\n";
	tstart = time(0);
	experiment_randomnns(&tree, points, num_trials, base);
	tend = time(0);
	cout << "Used: "<< difftime(tend, tstart) << "s\n";
}


/*void testMultipleDimension(string pathname) {
	cout << "Lets test multiple dimension kd\n";
	int dimension = 3;
	int fanout = 2; // XXX fanout must be 2 at this point!!!
	int num_trials = 20;
	int base = 200;
	config_t config(
			dimension, 
			BY_PERCENTILE,
			0.5, // Let half of the tree in memory
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
}*/


int main(int argc, char** argv) {
	if (argc != 2) {
		cout << "Please define a data source\n";
		exit(1);
	}
	string filename(argv[1]);
	cout << "build tree from " << filename << "\n";
//	testInsertRemove();
	testSingleDimension(filename);
//	testMultipleDimension(filename);
	return 0;
}