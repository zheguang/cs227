#include <ctime>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include "mfkd-tree.hpp"
#include "kd-tree.hpp"
using std::vector;
using std::cout;


int main(int argc, char** argv) {
	if (argc != 5) {
		cout << "Usage: <path> <dimension> <fanout> <range>\n";
		exit(1);
	}
	string filename(argv[1]);
	cout << "build tree from " << filename << "\n";
	int dimension = atoi(argv[2]);
	int fanout = atoi(argv[3]);
	int range = atoi(argv[4]);
	int numsearches = atoi(argv[5]);
	vector<tuple_t> points = createTuplesFromFile(filename, dimension); 
	
	config_t kd_config(dimension, BY_PERCENTILE, 0, 2);
	config_t mfkd_config(dimension, BY_PERCENTILE, 0, fanout);
	tree_t kd_tree(kd_config);
	csmftree_t mfkd_tree(mfkd_config);

	// Test insert
	cout << "start tree insert\n";
	time_t tstart = time(0);
//	for (unsigned int i = 0; i < points.size(); i++) {
	for (unsigned int i = 0; i < 20000; i++) {
		kd_tree.insert(points[i], hmindex::HybridMemory::DRAM);
	}
	time_t tend = time(0);
	cout << "finished kd tree insert. Used ";
	cout << difftime(tend, tstart) << "s\n";
	tstart = time(0);
	cout << "start csmf tree insert\n";
//	for (unsigned int i = 0; i < points.size(); i++) {
	for (unsigned int i = 0; i < 20000; i++) {
		mfkd_tree.insert(points[i], hmindex::HybridMemory::DRAM);
	}
	tend = time(0);
	cout << "finished cache sensitive nary kd tree insert. Used ";
	cout << difftime(tend, tstart) << "s\n";
	
	for (int i = 0; i < numsearches; i++) {
		tuple_t target = generate_tuple(dimension, range);		
		kd_tree.search_nearest(target);
	}
	for (int i = 0; i < numsearches; i++) {
		tuple_t target = generate_tuple(dimension, range);		
		datatype_t sdist;
		int index;
		mfkd_tree.search_nearest(target, index, sdist);
	}

	return 0;
}
