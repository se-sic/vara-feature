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
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
