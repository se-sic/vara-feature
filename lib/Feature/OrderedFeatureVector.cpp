#include "vara/Feature/OrderedFeatureVector.h"

#include <iterator>

namespace vara::feature {
void OrderedFeatureVector::insert(Feature *F) {
  Features.insert(
      std::upper_bound(Features.begin(), Features.end(), F,
                       [](vara::feature::Feature *A,
                          vara::feature::Feature *B) { return *A < *B; }),
      F);
}
} // namespace vara::feature
