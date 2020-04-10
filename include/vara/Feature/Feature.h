#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "llvm/ADT/SmallVector.h"

namespace vara::feature {

class Feature {
  llvm::SmallVector<int, 8> data;

public:
  void doStuff();
  void addStuff(int i);
  size_t getSize() const { return data.size(); }
  int doStuffer() const {
    int counter = 0;
    for (auto Iter = data.begin(); Iter != data.end(); ++Iter) {
      counter += *Iter;
    }
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
