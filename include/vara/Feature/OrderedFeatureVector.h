#ifndef VARA_FEATURE_ORDEREDFEATUREVECTOR_H
#define VARA_FEATURE_ORDEREDFEATUREVECTOR_H

#include "vara/Feature/Feature.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                            OrderedFeatureVector Class
//===----------------------------------------------------------------------===//

/// \brief Always ordered vector of features comparing with \a
/// Feature::operator<.
class OrderedFeatureVector {
public:
  using ordered_feature_iterator = typename std::vector<Feature *>::iterator;
  using const_ordered_feature_iterator =
      typename std::vector<Feature *>::const_iterator;

  OrderedFeatureVector() = default;
  template <class FeatureIterTy>
  OrderedFeatureVector(FeatureIterTy Start, FeatureIterTy End) {
    insert(llvm::iterator_range(std::move(Start), std::move(End)));
  }

  /// Insert feature while preserving ordering.
  void insert(Feature *F);

  void insert(const OrderedFeatureVector &OFV) {
    for (const auto &F : OFV) {
      insert(F);
    }
  }

  template <typename... Args> void insert(Feature *F, Args... FF) {
    insert(F);
    insert(FF...);
  }

  template <typename T> void insert(llvm::iterator_range<T> Iter) {
    for (const auto &Ptr : Iter) {
      if (auto *F = llvm::dyn_cast<Feature>(Ptr); F) {
        insert(F);
      }
    }
  }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  ordered_feature_iterator begin() { return Features.begin(); }
  [[nodiscard]] const_ordered_feature_iterator begin() const {
    return Features.begin();
  }

  ordered_feature_iterator end() { return Features.end(); }
  [[nodiscard]] const_ordered_feature_iterator end() const {
    return Features.end();
  }

private:
  std::vector<Feature *> Features;
};
} // namespace vara::feature

#endif // VARA_FEATURE_ORDEREDFEATUREVECTOR_H
