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
  using ordered_feature_iterator =
      typename llvm::SmallVector<Feature *, 3>::iterator;
  using const_ordered_feature_iterator =
      typename llvm::SmallVector<Feature *, 3>::const_iterator;

  OrderedFeatureVector() = default;
  OrderedFeatureVector(std::initializer_list<Feature *> Init) { insert(Init); }
  template <class FeatureIterTy>
  OrderedFeatureVector(FeatureIterTy &&Begin, FeatureIterTy &&End) {
    insert(std::forward<FeatureIterTy>(Begin),
           std::forward<FeatureIterTy>(End));
  }
  OrderedFeatureVector(const OrderedFeatureVector &OFV) = delete;
  OrderedFeatureVector &operator=(const OrderedFeatureVector &) = delete;
  OrderedFeatureVector(OrderedFeatureVector &&) = delete;
  OrderedFeatureVector &operator=(OrderedFeatureVector &&) = delete;
  ~OrderedFeatureVector() = default;

  /// Insert feature while preserving ordering.
  void insert(Feature *F);

  /// Remove feature.
  void remove(Feature *F) {
    Features.erase(std::remove(Features.begin(), Features.end(), F),
                   Features.end());
  }

  template <class FeatureIterTy>
  void insert(llvm::iterator_range<FeatureIterTy> Iter) {
    for (const auto &F : Iter) {
      insert(F);
    }
  }

  template <class FeatureIterTy>
  void insert(FeatureIterTy &&Begin, FeatureIterTy &&End) {
    insert(llvm::make_range(std::forward<FeatureIterTy>(Begin),
                            std::forward<FeatureIterTy>(End)));
  }

  void insert(std::initializer_list<Feature *> Init) {
    insert(Init.begin(), Init.end());
  }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  [[nodiscard]] bool empty() { return Features.empty(); }

  ordered_feature_iterator begin() { return Features.begin(); }
  [[nodiscard]] const_ordered_feature_iterator begin() const {
    return Features.begin();
  }

  ordered_feature_iterator end() { return Features.end(); }
  [[nodiscard]] const_ordered_feature_iterator end() const {
    return Features.end();
  }

private:
  llvm::SmallVector<Feature *, 5> Features;
};
} // namespace vara::feature

#endif // VARA_FEATURE_ORDEREDFEATUREVECTOR_H
