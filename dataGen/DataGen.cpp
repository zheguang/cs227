/*
 * Copyright (c) 2015 Brown University. All rights reserved.
 */
#include <algorithm>
#include <cstddef>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <string>
#include <climits>
#include <cstdlib>

#include "GenCommon.hpp"
#include "DataDef.hpp"

using std::size_t;
using std::string;
using std::vector;
using std::istream;
using std::getline;
using std::stringstream;
using std::ostringstream;
using std::ifstream;
using std::ofstream;
using std::stol;
using std::stoul;
using std::cerr;
using std::cout;
using std::rand;
using std::sort;
using std::malloc;

using namespace hmindex;

struct IntervalSpec {
  // (lower, upper]
  long lower;
  long upper;
  size_t frequency;
  double percentage;
};

struct Summary {
  size_t frequencyTotal;
};

vector<string> getNextLineAndSplitIntoTokens(istream& inputStream) {
  vector<string> result;
  string line;
  getline(inputStream, line);

  stringstream lineStream(line);
  string cell;

  while (getline(lineStream, cell, ',')) {
    result.push_back(cell);
  }
  return result;
}

IntervalSpec nextIntervalSpecFrom(const vector<string>& nextLine) {
  assert(nextLine.size() == 3);
  IntervalSpec result = {
    nextLine.at(0).compare("NINF") == 0 ? LONG_MIN : stol(nextLine.at(0)),
    nextLine.at(1).compare("PINF") == 0 ? LONG_MAX : stol(nextLine.at(1)),
    stoul(nextLine.at(2)),
    0,
  };
  return result;
}

vector<IntervalSpec> buildIntervalSpecsFrom(ifstream& specFileStream) {
  vector<IntervalSpec> intervalSpecs;
  size_t frequencyTotal = 0;

  assert(specFileStream.is_open());
  while (specFileStream.good()) {
    vector<string> nextLine = getNextLineAndSplitIntoTokens(specFileStream);
    if (nextLine.size() == 0) {
      break;
    }
    IntervalSpec nextSpec = nextIntervalSpecFrom(nextLine);
    intervalSpecs.push_back(nextSpec);
    frequencyTotal += nextSpec.frequency;
  }

  for (IntervalSpec& spec : intervalSpecs) {
    spec.percentage = static_cast<double>(spec.frequency) / frequencyTotal;
  }

  return intervalSpecs;
}

void genDataBasedOn(ifstream& specFileStream, ofstream& dataFileStream, ofstream& summaryFileStream, const GenOption& genOption) {
  assert(specFileStream.is_open());
  assert(dataFileStream.is_open());
  assert(summaryFileStream.is_open());
  vector<IntervalSpec> intervalSpecs = buildIntervalSpecsFrom(specFileStream);

  size_t numTuples = genOption.tableSizeInMb * (1 << 20) / getTupleSize();

  size_t tupleIdIt = 0;
  for (const IntervalSpec& spec : intervalSpecs) {
    //cerr << "spec percentage: " << spec.percentage << "\n";
    size_t numTuplesInInterval = numTuples * spec.percentage;
    Key lo = (spec.lower == LLONG_MIN) ? 0 : spec.lower;
    Key hi = spec.upper;
    Key diff = hi - lo;

    summaryFileStream << "{ lo: " << lo << ", hi: " << hi << ", percentage: " << spec.percentage << ", numTuples: " << numTuplesInInterval << " }\n";
    
    vector<Key> ks;
    for (size_t i = 0; i < numTuplesInInterval; i++) {
      Key randKey = rand() % diff + lo + 1; // (lo+1, hi]
      ks.push_back(randKey);
    }
    sort(ks.begin(), ks.end());

    vector<Tuple> fs;
    for (const Key& k : ks) {
      fs.push_back({
        tupleIdIt,
        k,
      });
      tupleIdIt++;
    }

    for (const Tuple& f : fs) {
      dataFileStream.write(reinterpret_cast<const char*>(&f), getTupleSize());
    }
  }

  summaryFileStream << "{ tableSizeInMb: " << genOption.tableSizeInMb << ", numTuplesApprox: " << numTuples << ", numTuplesGenerated: " << tupleIdIt << " tableGeneratedSize: " << tupleIdIt * getTupleSize() << "}\n";
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    cerr << "Invalid options. Usage: ./DataGen.out <tableSizeInMb> <specFilePath> <expName>\n";
    return 1;
  }
  const char* tableSizeInMb = argv[1];
  const char* specFilePath = argv[2];
  const char* expName = argv[3];
  ostringstream dataFileNameBuilder;
  ostringstream dataSummaryFileNameBuilder;
  dataFileNameBuilder << expName << "-" << argv[1] << "MB" << ".dat";
  dataSummaryFileNameBuilder << expName << "-" << argv[1] << "MB" << ".sum";
  GenOption genOption = { 
    stoul(tableSizeInMb),
    specFilePath,
    dataFileNameBuilder.str(),
    dataSummaryFileNameBuilder.str(),
  };
  assert(genOption.tableSizeInMb > 0 && genOption.tableSizeInMb <= 1 << 24);

  ofstream dataFile(genOption.dataFilePath, std::ios::out | std::ios::binary | std::ios::trunc);
  if (!dataFile.is_open()) {
    cerr << "Unable ot open data file: " << genOption.dataFilePath << "\n";
    return 1;
  }

  ofstream summaryFile(genOption.summaryFilePath, std::ios::out | std::ios::trunc);
  if (!summaryFile.is_open()) {
    cerr << "Unable ot open data file: " << genOption.dataFilePath << "\n";
    return 1;
  }
  
  ifstream specFile(specFilePath);
  if (!specFile.is_open()) {
    cerr << "Error: cannot open file: " << specFilePath << "\n";
    return 1;
  }

  genDataBasedOn(specFile, dataFile, summaryFile, genOption);

  summaryFile.close();
  specFile.close();
  dataFile.close();

  //printDataFile(genOption.dataFilePath);

  return 0;
}
