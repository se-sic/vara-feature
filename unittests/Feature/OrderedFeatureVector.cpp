#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {

class OrderedFeatureVectorTest : public ::testing::Test {
protected:
  void SetUp() override {
    FeatureModelBuilder B;
    B.makeFeature<BinaryFeature>("a");
    B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
    B.addEdge("a", "ab")->makeFeature<BinaryFeature>("ab");
    B.makeFeature<BinaryFeature>("b");
    B.addEdge("b", "ba")->makeFeature<BinaryFeature>("ba");
    B.addEdge("b", "bb")->makeFeature<BinaryFeature>("bb");
    FM = B.buildFeatureModel();
    assert(FM);
  }

  std::unique_ptr<FeatureModel> FM;
};

TEST_F(OrderedFeatureVectorTest, insert) {
  OrderedFeatureVector OFV;
  std::vector<Feature *> Expected = {
      FM->getFeature("bb"),  FM->getFeature("ba"), FM->getFeature("b"),
      FM->getFeature("ab"),  FM->getFeature("aa"), FM->getFeature("a"),
      FM->getFeature("root")};

  OFV.insert(FM->getFeature("a"));
  OFV.insert(FM->getFeature("root"));
  OFV.insert(FM->getFeature("ba"));
  OFV.insert(FM->getFeature("b"));
  OFV.insert(FM->getFeature("aa"));
  OFV.insert(FM->getFeature("ab"));
  OFV.insert(FM->getFeature("bb"));

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : FM->features()) {
    EXPECT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
}

TEST_F(OrderedFeatureVectorTest, insertFM) {
  OrderedFeatureVector OFV;
  std::vector<Feature *> Expected = {
      FM->getFeature("bb"),  FM->getFeature("ba"), FM->getFeature("b"),
      FM->getFeature("ab"),  FM->getFeature("aa"), FM->getFeature("a"),
      FM->getFeature("root")};

  OFV.insert(FM->features());

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : FM->features()) {
    EXPECT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
}

TEST_F(OrderedFeatureVectorTest, insertVariadic) {
  OrderedFeatureVector OFV;
  std::vector<Feature *> Expected = {
      FM->getFeature("bb"),  FM->getFeature("ba"), FM->getFeature("b"),
      FM->getFeature("ab"),  FM->getFeature("aa"), FM->getFeature("a"),
      FM->getFeature("root")};

  OFV.insert(
      {FM->getFeature("a"), FM->getFeature("root"), FM->getFeature("ab")});
  OFV.insert({FM->getFeature("aa"), FM->getFeature("b"), FM->getFeature("bb"),
              FM->getFeature("ba")});

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(Expected.size(), FM->size());
  for (const auto *F : FM->features()) {
    EXPECT_EQ(*Expected.back(), *F);
    Expected.pop_back();
  }
}

} // namespace vara::feature
