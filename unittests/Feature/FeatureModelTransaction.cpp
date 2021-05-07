#include "vara/Feature/FeatureModelTransaction.h"
#include "vara/Feature/FeatureModelBuilder.h"

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

  // Dummy method to fulfill the FeatureModelModification interface
  void exec(FeatureModel &_) override{};

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
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
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
    ASSERT_TRUE(FM);
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
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
}

TEST_F(FeatureModelTransactionCopyTest, addFeatureToModel) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  auto NewFM = FT.commit(); // Commit changes

  // Changes should not be visible on the old model
  EXPECT_FALSE(FM->getFeature("ab"));
  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));

  // Changes should be visible on the new model
  EXPECT_EQ(NewFM->size(), FM->size() + 1);
  EXPECT_TRUE(NewFM->getFeature("a"));
  EXPECT_TRUE(
      llvm::isa<RootFeature>(NewFM->getFeature("a")->getParentFeature()));
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
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.abort();

  auto NewFM = FT.commit(); // Commit changes, should not change anything and
                            // return no new FeatureModel

  EXPECT_EQ(NewFM.get(), nullptr);

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
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
    ASSERT_TRUE(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelTransactionModifyTest, createAndDestroyWithoutChange) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  // no change
  FT.commit();

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_NE(nullptr, FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
}

TEST_F(FeatureModelTransactionModifyTest, addFeatureToModel) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.commit(); // Commit changes

  EXPECT_EQ(FMSizeBefore + 1, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_TRUE(FM->getFeature("ab")); // Change should be visible
  EXPECT_EQ(FM->getFeature("a"), FM->getFeature("ab")->getParentFeature());
}

TEST_F(FeatureModelTransactionModifyTest, addFeatureToModelThenAboard) {
  size_t FMSizeBefore = FM->size();

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  FT.addFeature(std::make_unique<BinaryFeature>("ab"), FM->getFeature("a"));

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible

  FT.abort();

  FT.commit(); // Commit changes, should not change anything

  EXPECT_EQ(FMSizeBefore, FM->size());
  EXPECT_TRUE(FM->getFeature("a"));
  EXPECT_TRUE(llvm::isa<RootFeature>(FM->getFeature("a")->getParentFeature()));
  EXPECT_FALSE(FM->getFeature("ab")); // Change should not be visible
}

//===----------------------------------------------------------------------===//
//                    FeatureModelMergeTransaction Tests
//===----------------------------------------------------------------------===//

class FeatureModelMergeTransactionTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a", true);
    FM = B.buildFeatureModel();
    ASSERT_TRUE(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelMergeTransactionTest, Simple) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("b", true);
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  ASSERT_TRUE(FMMerged);

  Feature *F1 = FMMerged->getFeature("a");
  EXPECT_TRUE(F1);
  Feature *F2 = FMMerged->getFeature("b");
  EXPECT_TRUE(F2);
  EXPECT_EQ(Feature::FeatureKind::FK_ROOT, F1->getParentFeature()->getKind());
  EXPECT_EQ(Feature::FeatureKind::FK_ROOT, F2->getParentFeature()->getKind());
}

TEST_F(FeatureModelMergeTransactionTest, Idempotence) {
  size_t FMSizeBefore = FM->size();

  auto FMMerged = mergeFeatureModels(*FM, *FM);
  ASSERT_TRUE(FMMerged);

  EXPECT_EQ(FMSizeBefore, FMMerged->size());
  auto *F = FMMerged->getFeature("a");
  EXPECT_TRUE(F);
  EXPECT_EQ(Feature::FeatureKind::FK_BINARY, F->getKind());
}

TEST_F(FeatureModelMergeTransactionTest, DifferentLocations) {
  FeatureSourceRange FSR1("path", {2, 4}, {2, 30});
  FM->getFeature("a")->addLocation(FSR1);

  FeatureSourceRange FSR2("path", {10, 4}, {10, 30});
  FM->getFeature("a")->addLocation(FSR2);

  FeatureSourceRange FSR3("path", {12, 4}, {12, 30});
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", true,
                               std::vector<FeatureSourceRange>{FSR1, FSR3});
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  ASSERT_TRUE(FMMerged);

  Feature *F = FMMerged->getFeature("a");
  EXPECT_EQ(3, std::distance(F->getLocationsBegin(), F->getLocationsEnd()));
  EXPECT_NE(F->getLocationsEnd(),
            std::find(F->getLocationsBegin(), F->getLocationsEnd(), FSR1));
  EXPECT_NE(F->getLocationsEnd(),
            std::find(F->getLocationsBegin(), F->getLocationsEnd(), FSR2));
  EXPECT_NE(F->getLocationsEnd(),
            std::find(F->getLocationsBegin(), F->getLocationsEnd(), FSR3));
}

TEST_F(FeatureModelMergeTransactionTest, MultipleLevels) {
  size_t FMSizeBefore = FM->size();

  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("b", true);
  B.addEdge("b", "ba");
  B.addEdge("b", "bb");
  B.makeFeature<BinaryFeature>("ba", false);
  B.makeFeature<BinaryFeature>("bb", false);
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  ASSERT_TRUE(FMMerged);

  EXPECT_EQ(FMSizeBefore + 3, FMMerged->size());
  EXPECT_TRUE(FMMerged->getFeature("a"));
  EXPECT_TRUE(FMMerged->getFeature("b"));
  Feature *FBA = FMMerged->getFeature("ba");
  EXPECT_TRUE(FBA);
  EXPECT_EQ("b", FBA->getParentFeature()->getName());
  Feature *FBB = FMMerged->getFeature("bb");
  EXPECT_TRUE(FBB);
  EXPECT_EQ("b", FBB->getParentFeature()->getName());
}

TEST_F(FeatureModelMergeTransactionTest, RejectDifferenceOptional) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", false);
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  // Expect fail, property optional is different
  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  EXPECT_FALSE(FMMerged);
}

TEST_F(FeatureModelMergeTransactionTest, RejectDifferenceParent) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("b", true);
  B.addEdge("b", "a");
  B.makeFeature<BinaryFeature>("a", true);
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  // Expect fail, feature a has different parents; root vs. b
  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  EXPECT_FALSE(FMMerged);
}

TEST_F(FeatureModelMergeTransactionTest, RejectDifferenceKind) {
  FeatureModelBuilder B;
  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3}, true);
  auto FM2 = B.buildFeatureModel();
  ASSERT_TRUE(FM2);

  // Expect fail, feature a has different kinds; binary vs. numeric
  auto FMMerged = mergeFeatureModels(*FM, *FM2);
  EXPECT_FALSE(FMMerged);
}

//===----------------------------------------------------------------------===//
//                    FeatureModelRelationshipTransaction Tests
//===----------------------------------------------------------------------===//

class FeatureModelRelationshipTransactionTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a", true);
    B.addEdge("a", "aa");
    B.addEdge("a", "ab");
    B.makeFeature<BinaryFeature>("aa", false);
    B.makeFeature<BinaryFeature>("ab", false);
    FM = B.buildFeatureModel();
    ASSERT_TRUE(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelRelationshipTransactionTest, ModifyTransactionAddXorGroup) {
  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  Feature *A = FM->getFeature("a");
  auto Children = A->getChildren<Feature>();
  FT.addRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, A);
  // no visible changes
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }

  FT.commit();
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(1, F->getChildren<Relationship>().size());
    auto *R = *F->getChildren<Relationship>().begin();
    EXPECT_EQ(Relationship::RelationshipKind::RK_ALTERNATIVE, R->getKind());

    if (auto *RParent = llvm::dyn_cast_or_null<Feature>(R->getParent())) {
      ASSERT_EQ(*F, *RParent);
    } else {
      FAIL();
    }

    EXPECT_EQ(2, F->getChildren<Feature>().size());
    EXPECT_EQ(2, R->getChildren<Feature>().size());
    for (auto *FChild : R->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
}

TEST_F(FeatureModelRelationshipTransactionTest, CopyTransactionAddXorGroup) {
  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  Feature *A = FM->getFeature("a");
  auto Children = A->getChildren<Feature>();
  FT.addRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, A);
  // FM unchanged
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }

  auto NewFM = FT.commit();
  {
    auto *F = NewFM->getFeature("a");
    EXPECT_EQ(1, F->getChildren<Relationship>().size());
    auto *R = *F->getChildren<Relationship>().begin();
    EXPECT_EQ(Relationship::RelationshipKind::RK_ALTERNATIVE, R->getKind());

    if (auto *RParent = llvm::dyn_cast_or_null<Feature>(R->getParent())) {
      ASSERT_EQ(*F, *RParent);
    } else {
      FAIL();
    }

    EXPECT_EQ(2, F->getChildren<Feature>().size());
    EXPECT_EQ(2, R->getChildren<Feature>().size());
    for (auto *FChild : R->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
  // old FM still unchanged
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
}

TEST_F(FeatureModelRelationshipTransactionTest, CopyTransactionAddOrGroup) {
  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  Feature *A = FM->getFeature("a");
  auto Children = A->getChildren<Feature>();
  FT.addRelationship(Relationship::RelationshipKind::RK_OR, A);
  // FM unchanged
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }

  auto NewFM = FT.commit();
  {
    auto *F = NewFM->getFeature("a");
    EXPECT_EQ(1, F->getChildren<Relationship>().size());
    auto *R = *F->getChildren<Relationship>().begin();
    EXPECT_EQ(Relationship::RelationshipKind::RK_OR, R->getKind());

    if (auto *RParent = llvm::dyn_cast_or_null<Feature>(R->getParent())) {
      ASSERT_EQ(*F, *RParent);
    } else {
      FAIL();
    }

    EXPECT_EQ(2, F->getChildren<Feature>().size());
    EXPECT_EQ(2, R->getChildren<Feature>().size());
    for (auto *FChild : R->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
  // old FM still unchanged
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
}

TEST_F(FeatureModelRelationshipTransactionTest, ModifyTransactionAddOrGroup) {
  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  Feature *A = FM->getFeature("a");
  auto Children = A->getChildren<Feature>();
  FT.addRelationship(Relationship::RelationshipKind::RK_OR, A);
  // FM unchanged
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(0, F->getChildren<Relationship>().size());
    EXPECT_EQ(2, F->getChildren<Feature>().size());
    for (auto *FChild : F->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }

  FT.commit();
  {
    auto *F = FM->getFeature("a");
    EXPECT_EQ(1, F->getChildren<Relationship>().size());
    auto *R = *F->getChildren<Relationship>().begin();
    EXPECT_EQ(Relationship::RelationshipKind::RK_OR, R->getKind());

    if (auto *RParent = llvm::dyn_cast_or_null<Feature>(R->getParent())) {
      ASSERT_EQ(*F, *RParent);
    } else {
      FAIL();
    }

    EXPECT_EQ(2, F->getChildren<Feature>().size());
    EXPECT_EQ(2, R->getChildren<Feature>().size());
    for (auto *FChild : R->getChildren<Feature>()) {
      EXPECT_EQ(F, FChild->getParentFeature());
    }
  }
}

TEST_F(FeatureModelRelationshipTransactionTest, CopyTransactionRemoveOrGroup) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", true);
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);
  FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto V = detail::FeatureVariantTy(FM->getFeature("a"));
  FT.removeRelationship(V);
  EXPECT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  ASSERT_TRUE(A);
  EXPECT_EQ(0, A->getChildren<Relationship>().size());
  EXPECT_EQ(2, A->getChildren<Feature>().size());
  EXPECT_EQ(A, FM2->getFeature("aa")->getParent());
  EXPECT_EQ(A, FM2->getFeature("ab")->getParent());
}

TEST_F(FeatureModelRelationshipTransactionTest,
       CopyTransactionRemoveEmptyOrGroup) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", true);
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto V = detail::FeatureVariantTy(FM->getFeature("a"));
  FT.removeRelationship(V);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  ASSERT_TRUE(A);
  EXPECT_EQ(0, A->getChildren<Relationship>().size());
  EXPECT_EQ(0, A->getChildren<Feature>().size());
}

TEST_F(FeatureModelRelationshipTransactionTest,
       ModifyTransactionRemoveOrGroup) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", true);
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  B.addEdge("a", "aa");
  B.addEdge("a", "ab");
  B.makeFeature<BinaryFeature>("aa", false);
  B.makeFeature<BinaryFeature>("ab", false);
  FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  auto V = detail::FeatureVariantTy(FM->getFeature("a"));
  FT.removeRelationship(V);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  ASSERT_TRUE(FT.commit());

  auto *A = FM->getFeature("a");
  ASSERT_TRUE(A);
  EXPECT_EQ(0, A->getChildren<Relationship>().size());
  EXPECT_EQ(2, A->getChildren<Feature>().size());
  EXPECT_EQ(A, FM->getFeature("aa")->getParent());
  EXPECT_EQ(A, FM->getFeature("ab")->getParent());
}

TEST_F(FeatureModelRelationshipTransactionTest,
       ModifyTransactionRemoveEmptyOrGroup) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a", true);
  B.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "a");
  FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  auto V = detail::FeatureVariantTy(FM->getFeature("a"));
  FT.removeRelationship(V);
  ASSERT_EQ(1, FM->getFeature("a")->getChildren<Relationship>().size());

  ASSERT_TRUE(FT.commit());

  auto *A = FM->getFeature("a");
  ASSERT_TRUE(A);
  EXPECT_EQ(0, A->getChildren<Relationship>().size());
  EXPECT_EQ(0, A->getChildren<Feature>().size());
}

//===----------------------------------------------------------------------===//
//                    FeatureModelLocationsTransactionTest
//===----------------------------------------------------------------------===//

class FeatureModelLocationsTransactionTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder FB;
    FB.makeFeature<BinaryFeature>("a", true);
    FB.makeFeature<BinaryFeature>("b", true);
    FM = FB.buildFeatureModel();
    ASSERT_TRUE(FM);

    auto *B = FM->getFeature("b");
    B->addLocation(FSRInitial);
  }

  FeatureSourceRange FSRInitial{"path", {2, 4}, {2, 10}};
  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelLocationsTransactionTest, CopyTransactionAddLocation) {
  FeatureSourceRange FSRA("path", {2, 4}, {2, 10});
  FeatureSourceRange FSRB("path", {5, 4}, {5, 10});

  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto VA = detail::FeatureVariantTy(FM->getFeature("a"));
  auto VB = detail::FeatureVariantTy(FM->getFeature("b"));

  FT.addLocation(VA, FSRA);
  FT.addLocation(VB, FSRB);

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  auto *B = FM2->getFeature("b");
  EXPECT_EQ(1, std::distance(A->getLocationsBegin(), A->getLocationsEnd()));
  EXPECT_EQ(FSRA, *A->getLocationsBegin());
  EXPECT_EQ(2, std::distance(B->getLocationsBegin(), B->getLocationsEnd()));
  EXPECT_TRUE(std::find(B->getLocationsBegin(), B->getLocationsEnd(), FSRB) !=
              B->getLocationsEnd());
  EXPECT_TRUE(std::find(B->getLocationsBegin(), B->getLocationsEnd(),
                        FSRInitial) != B->getLocationsEnd());
}

TEST_F(FeatureModelLocationsTransactionTest, CopyTransactionRemoveLocation) {
  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto VB = detail::FeatureVariantTy(FM->getFeature("b"));

  FT.removeLocation(VB, FSRInitial);

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  auto *B = FM2->getFeature("b");
  EXPECT_EQ(0, std::distance(A->getLocationsBegin(), A->getLocationsEnd()));
  EXPECT_EQ(0, std::distance(B->getLocationsBegin(), B->getLocationsEnd()));
}

TEST_F(FeatureModelLocationsTransactionTest, ModifyTransactionAddLocation) {
  FeatureSourceRange FSRA("path", {2, 4}, {2, 10});
  FeatureSourceRange FSRB("path", {5, 4}, {5, 10});

  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  auto VA = detail::FeatureVariantTy(FM->getFeature("a"));
  auto VB = detail::FeatureVariantTy(FM->getFeature("b"));

  FT.addLocation(VA, FSRA);
  FT.addLocation(VB, FSRB);

  ASSERT_TRUE(FT.commit());

  auto *A = FM->getFeature("a");
  auto *B = FM->getFeature("b");
  EXPECT_EQ(1, std::distance(A->getLocationsBegin(), A->getLocationsEnd()));
  EXPECT_EQ(FSRA, *A->getLocationsBegin());
  EXPECT_EQ(2, std::distance(B->getLocationsBegin(), B->getLocationsEnd()));
  EXPECT_TRUE(std::find(B->getLocationsBegin(), B->getLocationsEnd(), FSRB) !=
              B->getLocationsEnd());
  EXPECT_TRUE(std::find(B->getLocationsBegin(), B->getLocationsEnd(),
                        FSRInitial) != B->getLocationsEnd());
}

//===----------------------------------------------------------------------===//
//                    FeatureModelRemoveFeatureTransactionTest
//===----------------------------------------------------------------------===//

class FeatureModelRemoveFeatureTransactionTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder FB;
    FB.makeFeature<BinaryFeature>("a", true);
    FB.makeFeature<BinaryFeature>("b", true);
    FB.makeFeature<BinaryFeature>("c", true);
    FB.addEdge("b", "ba");
    FB.addEdge("b", "bb");
    FB.makeFeature<BinaryFeature>("ba");
    FB.makeFeature<BinaryFeature>("bb");
    FB.emplaceRelationship(Relationship::RelationshipKind::RK_OR, "c");
    FB.addEdge("c", "ca");
    FB.addEdge("c", "cb");
    FB.makeFeature<BinaryFeature>("ca");
    FB.makeFeature<BinaryFeature>("cb");
    FM = FB.buildFeatureModel();
    ASSERT_TRUE(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(FeatureModelRemoveFeatureTransactionTest, CopyTransactionRemoveFeature) {
  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto VA = detail::FeatureVariantTy(FM->getFeature("a"));

  FT.removeFeature(VA);
  EXPECT_TRUE(FM->getFeature("a"));

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  auto *B = FM2->getFeature("b");
  EXPECT_FALSE(A);
  EXPECT_TRUE(B);
}

TEST_F(FeatureModelRemoveFeatureTransactionTest,
       CopyTransactionRemoveFeatureInGroup) {
  auto FT = FeatureModelCopyTransaction::openTransaction(*FM);
  auto VCA = detail::FeatureVariantTy(FM->getFeature("ca"));

  FT.removeFeature(VCA);
  EXPECT_TRUE(FM->getFeature("ca"));

  auto FM2 = FT.commit();
  ASSERT_TRUE(FM2);

  auto *A = FM2->getFeature("a");
  auto *B = FM2->getFeature("b");
  auto *C = FM2->getFeature("c");
  EXPECT_EQ(0, A->getChildren<Feature>().size());
  EXPECT_EQ(2, B->getChildren<Feature>().size());
  EXPECT_EQ(1, C->getChildren<Feature>().size());
  EXPECT_EQ(1, C->getChildren<Relationship>().size());

  auto *CB = FM2->getFeature("cb");
  ASSERT_TRUE(CB);
  ASSERT_EQ(*C->getChildren<Relationship>().begin(), CB->getParent());
}

TEST_F(FeatureModelRemoveFeatureTransactionTest,
       ModifyTransactionRemoveFeature) {
  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  auto VA = detail::FeatureVariantTy(FM->getFeature("a"));

  FT.removeFeature(VA);
  EXPECT_TRUE(FM->getFeature("a"));

  ASSERT_TRUE(FT.commit());

  auto *A = FM->getFeature("a");
  auto *B = FM->getFeature("b");
  EXPECT_FALSE(A);
  EXPECT_TRUE(B);
}

TEST_F(FeatureModelRemoveFeatureTransactionTest,
       ModifyTransactionRemoveFeatureInGroup) {
  auto FT = FeatureModelModifyTransaction::openTransaction(*FM);
  auto VCA = detail::FeatureVariantTy(FM->getFeature("ca"));

  FT.removeFeature(VCA);
  EXPECT_TRUE(FM->getFeature("ca"));

  ASSERT_TRUE(FT.commit());

  auto *A = FM->getFeature("a");
  auto *B = FM->getFeature("b");
  auto *C = FM->getFeature("c");
  EXPECT_EQ(0, A->getChildren<Feature>().size());
  EXPECT_EQ(2, B->getChildren<Feature>().size());
  EXPECT_EQ(1, C->getChildren<Feature>().size());
  EXPECT_EQ(1, C->getChildren<Relationship>().size());

  auto *CB = FM->getFeature("cb");
  ASSERT_TRUE(CB);
  ASSERT_EQ(*C->getChildren<Relationship>().begin(), CB->getParent());
}

} // namespace vara::feature
