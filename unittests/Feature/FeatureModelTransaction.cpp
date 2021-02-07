#include "vara/Feature/FeatureModelTransaction.h"

#include "gtest/gtest.h"

#include <memory>

namespace vara::feature {
namespace detail {

//===----------------------------------------------------------------------===//
//                      FeatureModelModification Tests
//===----------------------------------------------------------------------===//

class FeatureModelModificationTest : public ::testing::Test,
                                     protected FeatureModelModification {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a");
    FM = B.buildFeatureModel();
  }

  // Dummy method to fullfill the FeatureModelModification interface
  void exec(FeatureModel &FM) override{};

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelModificationTest, addFeatureToModel_simpleAdd) {
  size_t FMSizeBefore = FM->size();
  auto AddMod = FeatureModelModification::make_modification<AddFeatureToModel>(
      std::make_unique<BinaryFeature>("aa"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_FALSE(FM->getFeature("aa"));

  AddMod(*FM);

  EXPECT_EQ(FMSizeBefore + 1, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_TRUE(FM->getFeature("aa"));
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("aa")->getParentFeature());
}

TEST_F(FeatureModelModificationTest, addFeatureToModel_alreadyPresent) {
  size_t FMSizeBefore = FM->size();
  auto AddMod = FeatureModelModification::make_modification<AddFeatureToModel>(
      std::make_unique<BinaryFeature>("a"), FM->getFeature("a"));

  EXPECT_FALSE(AddMod(*FM));

  EXPECT_EQ(FMSizeBefore, FM->size());
}

TEST_F(FeatureModelModificationTest, addFeatureToModel_twoSuccessivley) {
  size_t FMSizeBefore = FM->size();
  auto AddModA = FeatureModelModification::make_modification<AddFeatureToModel>(
      std::make_unique<BinaryFeature>("aa"), FM->getFeature("a"));
  auto AddModB = FeatureModelModification::make_modification<AddFeatureToModel>(
      std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_TRUE(AddModA(*FM));
  EXPECT_TRUE(AddModB(*FM));

  EXPECT_EQ(FMSizeBefore + 2, FM->size());
  EXPECT_TRUE(FM->getFeature("aa"));
  EXPECT_TRUE(FM->getFeature("ab"));
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("aa")->getParentFeature());
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("ab")->getParentFeature());
}

} // namespace detail

//===----------------------------------------------------------------------===//
//                     FeatureModelCopyTransaction Tests
//===----------------------------------------------------------------------===//

class FeatureModelTransactionCopyTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a", true);
    FM = B.buildFeatureModel();
    assert(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelTransactionCopyTest, createAndDestroyWithoutChange) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  // no change
  FT.commit();
  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_NE(nullptr, FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
}

TEST_F(FeatureModelTransactionCopyTest, addFeatureToModel) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  auto NewFM = FT.commit(); // Commit changes

  // Changes should not be visible on the old model
  EXPECT_FALSE(FM->getFeature("ab"));
  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());

  // Changes should be visible on the new model
  EXPECT_EQ(NewFM->size(), FM->size() + 1);
  EXPECT_TRUE(NewFM->getFeature("a"));
  EXPECT_TRUE(NewFM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_TRUE(NewFM->getFeature("ab")); // Change should be visible
  EXPECT_EQ(NewFM->getFeature("a"),
            NewFM->getFeature("ab")->getParentFeature());
}

TEST_F(FeatureModelTransactionCopyTest, addFeatureToModelThenAboard) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.abort();

  auto NewFM = FT.commit(); // Commit changes, should not change anything and
                            // return no new FeatureModel

  EXPECT_EQ(NewFM.get(), nullptr);

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible
}

//===----------------------------------------------------------------------===//
//                    FeatureModelModifyTransaction Tests
//===----------------------------------------------------------------------===//

class FeatureModelTransactionModifyTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a", true);
    FM = B.buildFeatureModel();
    assert(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelTransactionModifyTest, createAndDestroyWithoutChange) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  // no change
  auto UpdatedFM = FT.commit();

  EXPECT_NE(FM, UpdatedFM);
  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_EQ(FMSizeBefore, UpdatedFM->size());
  EXPECT_NE(nullptr, UpdatedFM->getFeature("a"));
  EXPECT_TRUE(UpdatedFM->getFeature("a")->getParentFeature()->isRoot());
}

TEST_F(FeatureModelTransactionModifyTest, addFeatureToModel) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.commit(); // Commit changes

  EXPECT_EQ(FMSizeBefore + 1, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_TRUE(FM->getFeature("ab")); // Change should be visible
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("ab")->getParentFeature());
}

TEST_F(FeatureModelTransactionModifyTest, addFeatureToModelThenAboard) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.abort();

  FT.commit(); // Commit changes, should not change anything

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(FM->getFeature("a")->getParentFeature()->isRoot());
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible
}

} // namespace vara::feature
