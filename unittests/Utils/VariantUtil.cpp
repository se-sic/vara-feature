#include "vara/Utils/VariantUtil.h"

#include "gtest/gtest-death-test.h"
#include "gtest/gtest.h"

#include <limits>
#include <variant>
#include <vector>

namespace {
//===----------------------------------------------------------------------===//
// Variant Overloaded

TEST(TestOverloaded, VisitVariant) {
  std::vector<std::variant<int, double, bool>> Variants = {42, 21.42, false};
  for (auto &Var : Variants) {
    std::visit(Overloaded{[](auto Arg) { EXPECT_EQ(false, Arg); },
                          [](int Arg) { EXPECT_EQ(42, Arg); },
                          [](double Arg) { EXPECT_EQ(21.42, Arg); }},
               Var);
  }
}

TEST(TestNarrowingConversion, NarrowValueWithinBounds) {
  ASSERT_EQ(checkedNarrowingSignConversion(42), 42);
  ASSERT_EQ(checkedNarrowingSignConversion(std::numeric_limits<int>::max()),
            std::numeric_limits<int>::max());
  ASSERT_EQ(
      checkedNarrowingSignConversion(std::numeric_limits<int>::max() - 42),
      std::numeric_limits<int>::max() - 42);
}

TEST(TestNarrowingConversion, NarrowToLargeValue) {
#ifdef NDEBUG
  ASSERT_EQ(
      checkedNarrowingSignConversion(std::numeric_limits<unsigned>::max() - 42),
      std::numeric_limits<int>::max());
  ASSERT_EQ(
      checkedNarrowingSignConversion(std::numeric_limits<unsigned>::max()),
      std::numeric_limits<int>::max());
#else
  EXPECT_DEATH( // NOLINT
      {
        [[maybe_unused]] int _ = checkedNarrowingSignConversion(
            std::numeric_limits<unsigned>::max() - 42);
      },
      "Error: value to be narrowed was to large.");
  EXPECT_DEATH( // NOLINT
      {
        [[maybe_unused]] int _ = checkedNarrowingSignConversion(
            std::numeric_limits<unsigned>::max());
      },
      "Error: value to be narrowed was to large.");
#endif
}

} // namespace
