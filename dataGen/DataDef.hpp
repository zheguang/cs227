/*
 * Copyright (c) 2015 Brown University. All rights reserved.
 */
#ifndef VIS_INDEX_DEF_HPP
#define VIS_INDEX_DEF_HPP

#include <cstddef>

namespace hmindex {

  typedef long long Key;
  inline std::size_t getKeySize() {
    return sizeof(Key);
  }
  inline Key nextKeyOf(const Key& k) {
    return k + 1;
  }

  typedef size_t TupleId;
  inline std::size_t getTupleIdSize() {
    return sizeof(TupleId);
  }

  struct Tuple {
    TupleId tupleId;
    Key key;
  };

  size_t getTupleSize() {
    return sizeof(Tuple);
  }
}
#endif
