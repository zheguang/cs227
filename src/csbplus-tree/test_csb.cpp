#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "csb-tree.hpp"

using namespace std;

void testBulkload() {
	assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
    config_t config(
            12.0,4.0,
            BY_HEIGHT,
            1.0);
    bp_t bp_tree(config);
    bp_tree.addChildToRoot(bp_tree.g_root);

    bp_t::LPair test[1000];
    for (int i=0; i<1000; i++) {
        test[i].d_key=i;
        test[i].d_tid=i+1;
    }
    bp_tree.bulkload(1000, test, 11, 3);

    for (int i = 0; i < 1000; i++) {
      assert(bp_tree.search(bp_tree.g_root, i) == i + 1);
    }
    
}

void testInsertRemove() {
	cout << "Lets test B+ tree's insert and remove\n";
  //we need both types of nodes to have the same size
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_HEIGHT,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);
  int tempKey;
  void* tempChild;
  bp_t::LPair myPair;
  //insert into csbtree
  for (int i = 0; i < 1000; ++i)
  { 
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    bp_tree.insert(bp_tree.g_root, 0, 0, myPair, &tempKey, &tempChild, bp_tree.g_height);
  }
  //delete odd numbered tuples
  for (int i = 0; i < 1000; i++) {
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    if (i % 2 == 1)
    	bp_tree.csbdelete(bp_tree.g_root, myPair);
  }
  for (int i = 0; i < 1000; i++) {
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    //even numbered tuples should still exist.
    if (i % 2 == 0)
      assert(bp_tree.search(bp_tree.g_root, myPair.d_key) == i + 100);
    else //odd numbered tuples should be deleted, and thus equal 0
      assert(bp_tree.search(bp_tree.g_root, myPair.d_key) == 0);
	}
}

void testGetHeight() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_HEIGHT,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);
  //bulkloaded in 100 nodes
  bp_t::LPair test[1000];
  for (int i=0; i<1000; i++) {
      test[i].d_key=i;
      test[i].d_tid=i+1;
  }
  bp_tree.bulkload(1000, test, 11, 3);
  assert(bp_tree.get_height(bp_tree.g_root) == 3);
}

void testBY_HEIGHTbulkload() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_HEIGHT,
          1.0);
  bp_t bp_tree(config);

  bp_t::LPair test[10000];
  for (int i=0; i<10000; i++) {
      test[i].d_key=i;
      test[i].d_tid=i+1;
  }
  bp_tree.bulkload(10000, test, 11, 3);
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("DRAM/DRAM/DRAM/NVM/NVM/");
  assert(str1.compare(str2) == 0);
}

void testBY_TIER_DRAMbulkload() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_TIER_DRAM,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);
  bp_t::LPair test[10000];
  for (int i=0; i<10000; i++) {
      test[i].d_key=i;
      test[i].d_tid=i+1;
  }
  bp_tree.bulkload(10000, test, 11, 3);
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("NVM/DRAM/NVM/DRAM/DRAM/");
  assert(str1.compare(str2) == 0);
}

void testBY_TIER_NVMbulkload() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_TIER_NVM,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);

  bp_t::LPair test[10000];
  for (int i=0; i<10000; i++) {
      test[i].d_key=i;
      test[i].d_tid=i+1;
  }
  bp_tree.bulkload(10000, test, 11, 3);
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("DRAM/NVM/DRAM/NVM/NVM/");
  assert(str1.compare(str2) == 0);
  bp_tree.~bp_t();
}

void testBY_HEIGHTinsert() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_HEIGHT,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);

  int tempKey;
  void* tempChild;
  bp_t::LPair myPair;
  //insert into csbtree
  for (int i = 0; i < 10000; ++i)
  { 
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    bp_tree.insert(bp_tree.g_root, 0, 0, myPair, &tempKey, &tempChild, bp_tree.g_height);
  }
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("DRAM/DRAM/DRAM/NVM/NVM/");
  assert(str1.compare(str2) == 0);
}

void testBY_TIER_DRAMinsert() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_TIER_DRAM,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);
  int tempKey;
  void* tempChild;
  bp_t::LPair myPair;
  //insert into csbtree
  for (int i = 0; i < 10000; ++i)
  { 
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    bp_tree.insert(bp_tree.g_root, 0, 0, myPair, &tempKey, &tempChild, bp_tree.g_height);
  }
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("NVM/DRAM/NVM/DRAM/DRAM/");
  assert(str1.compare(str2) == 0);
}

void testBY_TIER_NVMinsert() {
  assert(sizeof(bp_t::CSBLNODE) == sizeof(bp_t::CSBINODE));
  config_t config(
          12.0,4.0,
          BY_TIER_NVM,
          1.0);
  bp_t bp_tree(config);
  bp_tree.addChildToRoot(bp_tree.g_root);

  int tempKey;
  void* tempChild;
  bp_t::LPair myPair;
  //insert into csbtree


  for (int i = 0; i < 10000; ++i)
  { 
    myPair.d_key = i;
    myPair.d_tid = i + 100;
    bp_tree.insert(bp_tree.g_root, 0, 0, myPair, &tempKey, &tempChild, bp_tree.g_height);
  }
  std::string str1 = bp_tree.check_levels(bp_tree.g_root);
  std::string str2 ("DRAM/NVM/DRAM/NVM/NVM/");
  assert(str1.compare(str2) == 0);
}

int main() {
	testInsertRemove();
	testBulkload();
  testBY_HEIGHTbulkload();
  testBY_TIER_DRAMbulkload();
  testBY_TIER_NVMbulkload();
  testBY_HEIGHTinsert();
  testBY_TIER_DRAMinsert();
  testBY_TIER_NVMinsert();
  cout << "All tests pass!\n";
	return 0;
}

