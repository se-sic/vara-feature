#include "vara/Feature/Feature.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(BinaryFeature, BinaryFeatureBasics) {
  auto P = vara::feature::XmlParser("../test/test.xml", "../vm.dtd");
}
} // namespace vara::feature
