#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(OrderedFeatureVector, insert) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"a", "aa"},
                                                           {"a", "ab"},
                                                           {"b", "bb"},
                                                           {"root", "b"},
                                                           {"root", "a"},
                                                           {"b", "ba"}});
  assert(FM);
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

TEST(OrderedFeatureVector, insertFM) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"a", "aa"},
                                                           {"a", "ab"},
                                                           {"b", "bb"},
                                                           {"root", "b"},
                                                           {"root", "a"},
                                                           {"b", "ba"}});
  assert(FM);
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

TEST(OrderedFeatureVector, insertVariadic) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"a", "aa"},
                                                           {"a", "ab"},
                                                           {"b", "bb"},
                                                           {"root", "b"},
                                                           {"root", "a"},
                                                           {"b", "ba"}});
  assert(FM);
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
