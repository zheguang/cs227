#include <cstdlib>
#include <cmath>

enum policy_t {
	BY_HEIGHT_FROM_BOTTOM,
	BY_PERCENTILE,
};

struct config_t {
	int dimension;
	policy_t policy;
	float value;
	int fanout;
	config_t(int d, policy_t p, float v, int f=2):
			dimension(d),
			policy(p),
			value(v),
			fanout(f) {}
};


// Suppose fanout is still 2
inline int bottomheight(int N) {
	return int(log2(N) + 1);
}


