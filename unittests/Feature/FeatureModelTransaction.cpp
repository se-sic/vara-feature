#include "vara/Feature/FeatureModelTransaction.h"

#include "gtest/gtest.h"

namespace vara::feature {

// TODO: rewrite tests to check transaction framework and modifications
// separatly

// TEST(FeatureModelTransactionCopy, createAndDestroyWithoutChange) {
//   // TODO: refactor this into a test fixture
//   FeatureModelBuilder B;
//   B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);
//   auto FM = B.buildFeatureModel();
//   size_t FMSizeBefore = FM->size();
//
//   auto FT = FeatureModelCopyTransaction::openTransaction(FM.get());
//   // no change
//   auto UpdatedFM = FT.commit();
//
//   EXPECT_EQ(FMSizeBefore, FM->size());
//   EXPECT_EQ(FMSizeBefore, UpdatedFM->size());
//   EXPECT_NE(nullptr, UpdatedFM->getFeature("a"));
//   EXPECT_TRUE(UpdatedFM->getFeature("a")->getParentFeature()->isRoot());
// }

TEST(FeatureModelTransactionMove, createAndDestroyWithoutChange) {
  // TODO: refactor this into a test fixture
  FeatureModelBuilder B;
  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);
  auto FM = B.buildFeatureModel();
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(FM.get());
  // no change
  FT.commit();
  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_NE(nullptr, FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
}

// TODO: Copy missing

TEST(FeatureModelTransactionMove, addFeatureToModel) {
  // TODO: refactor this into a test fixture
  FeatureModelBuilder B;
  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);
  auto FM = B.buildFeatureModel();
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(FM.get());
  FT.addFeature(std::make_unique<BinaryFeature>(
                    "ab", false, std::vector<FeatureSourceRange>()),
                FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_NE(nullptr, FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_EQ(nullptr, FM->getFeature("ab")); // Change should not be visible

  FT.commit(); // Commit changes

  EXPECT_EQ(FMSizeBefore + 1, FM->size());
  EXPECT_NE(nullptr, FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_NE(nullptr, FM->getFeature("ab")); // Change should be visible
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("ab")->getParentFeature());
}

} // namespace vara::feature
