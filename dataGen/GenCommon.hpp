/*
 * Copyright (c) 2015 Brown University. All rights reserved.
 */
#ifndef GEN_COMMON_HPP
#define GEN_COMMON_HPP

#include <cstddef>
#include <fstream>
#include <iostream>

#include "DataDef.hpp"

using std::string;
using std::ifstream;
using std::cerr;
using std::size_t;

namespace hmindex {

  const Key ONE_M = 1000 * 1000;
  const Key ONE_K = 1000;

  struct GenOption {
    size_t tableSizeInMb;
    string specFilePath;
    string dataFilePath;
    string summaryFilePath;
  };

  void printDataFile(const string& dataFilePath) {
    ifstream dataFile(dataFilePath);
    if (!dataFile.is_open()) {
      cerr << "Unable to print data file: " << dataFilePath << "\n";
      return;
    }

    dataFile.seekg(0, std::ios_base::end);
    size_t dataSize = dataFile.tellg();
    cerr << "{ DataFileSize: " << dataSize <<  "B, tupleSize: " << getTupleSize() << " }\n";
    dataFile.seekg(0);

    char* dataBlock = static_cast<char*>(malloc(dataSize));
    dataFile.read(dataBlock, dataSize);

    for (size_t i = 0; i < dataSize; i += getTupleSize()) {
      Tuple* f = reinterpret_cast<Tuple*>(dataBlock + i);
      cerr << "{ tupleId: " << f->tupleId << " key: " << f->key << " }\n";
    }

    dataFile.close();
  }

}
#endif
