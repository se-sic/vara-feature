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

  OFV.insert(FM->getFeature("a"));
  OFV.insert(FM->getFeature("root"));
  OFV.insert(FM->getFeature("ba"));
  OFV.insert(FM->getFeature("b"));
  OFV.insert(FM->getFeature("aa"));
  OFV.insert(FM->getFeature("ab"));
  OFV.insert(FM->getFeature("bb"));

  EXPECT_EQ(OFV.size(), FM->size());
  auto Iter = OFV.begin();
  EXPECT_EQ(**Iter++, *FM->getFeature("root"));
  EXPECT_EQ(**Iter++, *FM->getFeature("a"));
  EXPECT_EQ(**Iter++, *FM->getFeature("aa"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ab"));
  EXPECT_EQ(**Iter++, *FM->getFeature("b"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ba"));
  EXPECT_EQ(**Iter++, *FM->getFeature("bb"));
  EXPECT_EQ(Iter, OFV.end());
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

  OFV.insert(FM->features());

  EXPECT_EQ(OFV.size(), FM->size());
  auto Iter = OFV.begin();
  EXPECT_EQ(**Iter++, *FM->getFeature("root"));
  EXPECT_EQ(**Iter++, *FM->getFeature("a"));
  EXPECT_EQ(**Iter++, *FM->getFeature("aa"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ab"));
  EXPECT_EQ(**Iter++, *FM->getFeature("b"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ba"));
  EXPECT_EQ(**Iter++, *FM->getFeature("bb"));
  EXPECT_EQ(Iter, OFV.end());
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

  OFV.insert(FM->getFeature("a"), FM->getFeature("root"), FM->getFeature("ab"));
  OFV.insert(FM->getFeature("aa"), FM->getFeature("b"), FM->getFeature("bb"),
             FM->getFeature("ba"));

  EXPECT_EQ(OFV.size(), FM->size());
  auto Iter = OFV.begin();
  EXPECT_EQ(**Iter++, *FM->getFeature("root"));
  EXPECT_EQ(**Iter++, *FM->getFeature("a"));
  EXPECT_EQ(**Iter++, *FM->getFeature("aa"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ab"));
  EXPECT_EQ(**Iter++, *FM->getFeature("b"));
  EXPECT_EQ(**Iter++, *FM->getFeature("ba"));
  EXPECT_EQ(**Iter++, *FM->getFeature("bb"));
  EXPECT_EQ(Iter, OFV.end());
}
} // namespace vara::feature
