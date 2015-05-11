// config for b plus tree
#ifndef BP_CONFIG_H_
#define BP_CONFIG_H_

#include <cstdlib>
#include <cmath>

enum policy_t {
	BY_HEIGHT,
	BY_TIER_DRAM,
	BY_TIER_NVM,
	DRAM
};

struct config_t {
	int keysize; //number of keys in internal node
	int leafsize; //number of keys in leaf node
	policy_t policy;
	config_t(int d, int e, policy_t p):
			keysize(d),
			leafsize(e),
			policy(p) {}
};

#endif
