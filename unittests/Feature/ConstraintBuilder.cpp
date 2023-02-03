#include "vara/Feature/ConstraintBuilder.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConstraintBuilder, build) {
  auto CB = ConstraintBuilder();
  CB.constant(1).implies().lNot().feature("Foo")();
  CB.implies().lNot().feature("Bar").implies().constant(4)();
  CB.implies().constant(5);

  auto C = CB.build();
  ASSERT_TRUE(C);

  llvm::outs() << C->toString() << '\n';
}

TEST(ConstraintBuilder, error) {
  auto CB = ConstraintBuilder();
  CB.constant(1).implies().excludes().constant(2);

  EXPECT_FALSE(CB.build());
}

} // namespace vara::feature
