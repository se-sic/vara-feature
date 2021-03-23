#include "vara/Feature/FeatureModelBuilder.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  if (!FM->getRoot()) {
    auto T = FeatureModelModifyTransaction::openTransaction(*FM);
    T.setRoot(std::make_unique<RootFeature>("root"));
    if (!T.commit()) {
      return nullptr;
    }
  }
  return Features.commit() && Transactions.commit() && PostTransactions.commit()
             ? std::move(FM)
             : nullptr;
}

} // namespace vara::feature
