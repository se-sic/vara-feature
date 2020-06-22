#ifndef VARA_FEATURE_ORDEREDFEATUREVECTOR_H
#define VARA_FEATURE_ORDEREDFEATUREVECTOR_H

#include "vara/Feature/Feature.h"

namespace vara::feature {

class OrderedFeatureVector {
public:
  using ordered_feature_iterator = typename std::vector<Feature *>::iterator;
  using const_ordered_feature_iterator =
      typename std::vector<Feature *>::const_iterator;

  OrderedFeatureVector() = default;

  void insert(Feature *F) {
    Features.insert(
        std::upper_bound(Features.begin(), Features.end(), F,
                         [](vara::feature::Feature *A,
                            vara::feature::Feature *B) { return *A < *B; }),
        F);
  }

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
