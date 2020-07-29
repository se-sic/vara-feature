#include <vara/Feature/FeatureModelWriter.h>
#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(XmlWriter, testtest) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "aba"}, {"root", "a"}});
  assert(FM);

  FeatureModelXmlWriter fmxw = FeatureModelXmlWriter(*FM);
  char *buf;
  int rc = fmxw.writeFeatureModel(&buf);
  printf("%s", buf);
  EXPECT_EQ(0, 1);
}

} // namespace vara::feature
