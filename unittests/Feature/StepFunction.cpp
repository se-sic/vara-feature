#include "vara/Feature/StepFunction.h"

#include "gtest/gtest.h"

namespace vara::feature {

template <typename T>
class StepFunctionTest : public ::testing::Test {
protected:
  static void checkEqual(T V1, T V2) { EXPECT_EQ(V1, V2); }

  static void checkStepFunction(T Start, StepFunction *S,
                                const std::function<T(T)> &F) {
    T Next = Start;
    T Expected = Start;
    for (int Step = 0; Step < STEPS; ++Step) {
      Next = S->next<T>(Next);
      Expected = F(Expected);
      checkEqual(Next, Expected);
    }
  }

private:
  static const int STEPS = 10;
};
template <>
void StepFunctionTest<float>::checkEqual(float V1, float V2) {
  EXPECT_FLOAT_EQ(V1, V2);
}
template <>
void StepFunctionTest<double>::checkEqual(double V1, double V2) {
  EXPECT_DOUBLE_EQ(V1, V2);
}

using types = ::testing::Types<double, float, long, int, unsigned int>;
TYPED_TEST_SUITE(StepFunctionTest, types, );

TYPED_TEST(StepFunctionTest, addition) {
  auto S = StepFunction(StepFunction::StepOperation::ADDITION, 13.37);
  TestFixture::checkStepFunction(-100, &S,
                                 [](TypeParam X) { return X + 13.37; });
}

TYPED_TEST(StepFunctionTest, multiplication) {
  auto S = StepFunction(StepFunction::StepOperation::MULTIPLICATION, -13.37);
  TestFixture::checkStepFunction(1, &S, [](TypeParam X) { return X * -13.37; });
}

TYPED_TEST(StepFunctionTest, exponentiationL) {
  auto S = StepFunction(StepFunction::StepOperation::EXPONENTIATION, 2);
  TestFixture::checkStepFunction(1, &S,
                                 [](TypeParam X) { return std::pow(X, 2); });
}

TYPED_TEST(StepFunctionTest, exponentiationR) {
  auto S = StepFunction(2, StepFunction::StepOperation::EXPONENTIATION);
  TestFixture::checkStepFunction(1, &S,
                                 [](TypeParam X) { return std::pow(2, X); });
}

TEST(StepFunction, commutative) {
  auto L = StepFunction(StepFunction::StepOperation::MULTIPLICATION, 13.37);
  auto R = StepFunction(13.37, StepFunction::StepOperation::MULTIPLICATION);

  EXPECT_DOUBLE_EQ(L(42), R(42));
}

TEST(StepFunction, next) {
  auto S = StepFunction(StepFunction::StepOperation::MULTIPLICATION, 13.37);

  EXPECT_DOUBLE_EQ(S.next<double>(42), 561.54);
  EXPECT_DOUBLE_EQ(S.next(42.), 561.54);
  EXPECT_EQ(S.next<int>(42.), 561);
  EXPECT_EQ(S.next(42), 561);
}

TEST(StepFunction, call) {
  auto S = StepFunction(StepFunction::StepOperation::MULTIPLICATION, 13.37);

  EXPECT_DOUBLE_EQ(S(42.), 561.54);
  EXPECT_DOUBLE_EQ(S(42), 561.54);
}

} // namespace vara::feature
