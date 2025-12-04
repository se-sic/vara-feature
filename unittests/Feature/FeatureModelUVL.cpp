#include "vara/Feature/FeatureModelParser.h"

#include "Utils/UnittestHelper.h"

#include "llvm/Support/MemoryBuffer.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace vara::feature {

std::unique_ptr<const FeatureModel> buildFeatureModelUVL(llvm::StringRef Path) {
  auto FS = llvm::MemoryBuffer::getFileAsStream(getTestResource(Path));
  assert(FS);
  auto P = FeatureModelUvlParser(FS.get()->getBuffer().str());
  assert(P.verifyFeatureModel());
  return P.buildFeatureModel();
}

TEST(FeatureModelParser, uvl_simple) {
  EXPECT_TRUE(buildFeatureModelUVL("test_constraints.uvl"));
}

TEST(FeatureModelParser, uvl_booleanConstraints) {

  auto C = OrConstraint(std::make_unique<PrimaryFeatureConstraint>(
                            std::make_unique<BinaryFeature>("A")),
                        std::make_unique<PrimaryFeatureConstraint>(
                            std::make_unique<BinaryFeature>("B")));

  auto FM = buildFeatureModelUVL("test_constraints.uvl");
  ASSERT_TRUE(FM);

  EXPECT_EQ(
      FM->booleanConstraints().begin()->constraint()->getRoot()->toString(),
      C.toString());
}

} // namespace vara::feature
