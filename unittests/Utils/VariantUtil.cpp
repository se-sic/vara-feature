#include "vara/Utils/VariantUtil.h"

#include "gtest/gtest.h"

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

} // namespace 
