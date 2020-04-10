#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "llvm/ADT/SmallVector.h"

namespace vara::feature {

class Feature {
  llvm::SmallVector<int, 8> Data;

public:
  void doStuff();
  void addStuff(int I);
  [[nodiscard]] size_t getSize() const { return Data.size(); }
  [[nodiscard]] int doStuffer() const {
    int Counter = 0;
    for (auto Value : Data) {
      Counter += Value;
    }
    return Counter;
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
