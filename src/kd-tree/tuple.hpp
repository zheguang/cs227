#include <sstream>
#include <cmath>
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::ostringstream;

typedef vector<double> tuple_t;

// Print a tuple_t
string tuple_string(tuple_t t);

// calculate norm2 distance between two points
double distance(tuple_t& t1, tuple_t& t2);

// Swap position of two tuples in the list
inline void swap_tuples_in_points(vector<tuple_t>& points, int i1, int i2) {
	tuple_t tmp = points[i1];
	points[i1] = points[i2];
	points[i2] = tmp;
}

// Return the index of median of points[lbd:rbd+1] using quick find by
// axis-th coordinates of each point
int quickfind_tuples_by_axis(
		vector<tuple_t>& points,
	  int lbd, int& rbd, unsigned int axis, int right_median);

// Create a list of tuple from .dat file created by dataGen
vector<tuple_t> createTuplesFromFile(
	const string& dataFilePath, int dimension);
