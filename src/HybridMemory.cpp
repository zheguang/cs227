/*
 * Copyright (c) 2015 Brown University.
 */
#include <numa.h>
#include <numaif.h>

#include "HybridMemory.hpp"
#include "FatalError.hpp"

#define PAGE_SIZE (1 << 12)
#define PAGE_MASK ~(PAGE_SIZE - 1)

using namespace hmindex;

void* HybridMemory::alloc(size_t sz, MEMORY_NODE_TYPE memoryNodeType) {
  void* result = numa_alloc_onnode(sz, memoryNodeOf(memoryNodeType));
  memset(result, 0, sz);
#ifdef HYBRID_MEMORY_CHECK
  assertAddress(result, memoryNodeType);
#endif
  return result;
}

void HybridMemory::free(void* start, size_t sz, MEMORY_NODE_TYPE memoryNodeType __attribute__((unused))) {
  numa_free(start, sz); 
}

void HybridMemory::assertAddress(void* start, MEMORY_NODE_TYPE memoryNodeType) {
  if (!start) {
    throwFatalError("Null start address.");
  }

  const int numObjs = 1;
  int status[numObjs];
  void *page[numObjs];
  page[0] = (void *)((unsigned long)start & PAGE_MASK);

  long rc = move_pages(0, numObjs, page, NULL, status, MPOL_MF_MOVE); // Get the status on current node.

  if (rc) {
    throwFatalError("Unexpected error in assert address: cannot move page.");
  }
  int memoryNode = memoryNodeOf(memoryNodeType);
  if (status[0] != memoryNode) {
    throwFatalError("Address error: not on the expected memory node. Expected: %d, actual: %d. Poiter: %p. Page: %p\n.", memoryNode, status[0], start, page[0]);
  }
}

int HybridMemory::memoryNodeOf(MEMORY_NODE_TYPE memoryNodeType) {
  int memoryNode = 0;
  switch (memoryNodeType) {
    case DRAM:
      memoryNode = 0;
      break;
    case NVM:
      memoryNode = 1;
      break;
    default:
      throwFatalError("Non supported memory node type");
  }
  return memoryNode;
}


