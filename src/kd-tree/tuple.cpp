#include <vector>
#include "tuple.hpp"
using std::vector;

string tuple_string(tuple_t t) {
	ostringstream buf;
	buf << "(";
	for (unsigned int i = 0; i < t.size(); i++) {
		buf << t[i] << " ";
	} buf << ")";
	return buf.str();
}

float distance(tuple_t& t1, tuple_t& t2) {
	if (t1.size() != t2.size()) return -float(1);
	float ret = 0.0;
	for (unsigned int i = 0; i < t1.size(); i++) {
		ret += (t1[i] - t2[i]) * (t1[i] - t2[i]);
	}
	return pow(ret, 0.5);
}

int quickfind_tuples_by_axis(
		vector<tuple_t>& points, int lbd, int& rbd, unsigned int axis, int right_median) {
	if (lbd == rbd) return lbd;
	tuple_t pivot = points[lbd];
	int i = lbd+1;
	for (int j = lbd+1; j <= rbd; j++) {
		tuple_t compare = points[j];
		if (compare.size() <= axis) {
			// Tuple compare is a invalid tuple as it doesn't have axis-th coordinate,
			// swap to the end, shrink valid tuples range
			swap_tuples_in_points(points, j, rbd);
			rbd--;
		} 
		if (compare[axis] < pivot[axis]) {
			swap_tuples_in_points(points, i, j);
			i++;
		}
	}
	i--;
	swap_tuples_in_points(points, lbd, i);
	if (right_median == i - lbd) {
		return i;
	}
	if (right_median < i - lbd) {
		int new_rbd = i-1;
	 	return quickfind_tuples_by_axis(points, lbd, new_rbd, axis, right_median);
	}
 	return quickfind_tuples_by_axis(points, i+1, rbd, axis, right_median-(i-lbd+1));	
}

