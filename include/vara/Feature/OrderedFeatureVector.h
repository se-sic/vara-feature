#ifndef VARA_FEATURE_FEATURESORT_H
#define VARA_FEATURE_FEATURESORT_H

#include "FeatureModel.h"

namespace vara::feature {

class OrderedFeatureVector {
public:
  using feature_heap_iterator = typename std::vector<Feature *>::iterator;
  using const_feature_heap_iterator =
      typename std::vector<Feature *>::const_iterator;

  template <typename T> OrderedFeatureVector(T FM) {
    for (auto F = FM->begin(); F != FM->end(); ++F) {
      Features.push_back(*F);
    }
    std::sort(Features.begin(), Features.end(),
              Feature::FeatureDepthFirstComparator());
  }

  feature_heap_iterator begin() { return Features.begin(); }

  [[nodiscard]] const_feature_heap_iterator begin() const {
    return Features.begin();
  }

  feature_heap_iterator end() { return Features.end(); }

  [[nodiscard]] const_feature_heap_iterator end() const {
    return Features.end();
  }

private:
  std::vector<Feature *> Features;
};

} // namespace vara::feature
#endif // VARA_FEATURE_FEATURESORT_H
