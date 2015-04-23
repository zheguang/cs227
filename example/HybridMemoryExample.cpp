/*
 * Copyright (c) 2015 Brown University.
 */
#include <cstddef>
#include <stdexcept>
#include <iostream>

#include "../src/HybridMemory.hpp"

using std::size_t;
using std::cerr;
using std::runtime_error;
using std::string;

using namespace hmindex;

const int KB = 1 << 10;
const int NUM_BLOCKS = 10;

string memoryNodeTypeNameOf(HybridMemory::MEMORY_NODE_TYPE memoryNodeType) {
  switch (memoryNodeType) {
    case HybridMemory::DRAM:
      return "DRAM";
    case HybridMemory::NVM:
      return "NVM";
    default:
      throw runtime_error("Unsupported node type.");
  };
}

void checkAllocationIn(HybridMemory::MEMORY_NODE_TYPE memoryNodeType) {
  void* blocks[NUM_BLOCKS];
  size_t blockSizes[NUM_BLOCKS];

  for (int i = 0; i < NUM_BLOCKS; i++) {
    blockSizes[i] = (i+1) * KB;
    cerr << "[INFO] Allocate block " << i << " of size " << blockSizes[i] << "B in " << memoryNodeTypeNameOf(memoryNodeType) << "\n";
    blocks[i] = HybridMemory::alloc(blockSizes[i], memoryNodeType);
  }

  for (int i = 0; i < NUM_BLOCKS; i++) {
    cerr << "[INFO] Check block " << i << " of size " << blockSizes[i] << "B in " << memoryNodeTypeNameOf(memoryNodeType) << "\n";
    HybridMemory::assertAddress(blocks[i], memoryNodeType);
  }

  for (int i = 0; i < NUM_BLOCKS; i++) {
    cerr << "[INFO] Free block " << i << " of size " << blockSizes[i] << "B in " << memoryNodeTypeNameOf(memoryNodeType) << "\n";
    HybridMemory::free(blocks[i], blockSizes[i], memoryNodeType);
  }
}

int main() {
  checkAllocationIn(HybridMemory::DRAM);
  checkAllocationIn(HybridMemory::NVM);

  return 0;
}
