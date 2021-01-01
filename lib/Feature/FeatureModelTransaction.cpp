#include "vara/Feature/FeatureModelTransaction.h"

#include <iostream>

namespace vara::feature {

void addFeature(FeatureModel *FM, std::unique_ptr<Feature> NewFeature,
                Feature *Parent) {
  auto Trans = FeatureModelModifyTransaction::openTransaction(FM);
  Trans.addFeature(std::move(NewFeature), Parent);
  Trans.commit();
}

} // namespace vara::feature
