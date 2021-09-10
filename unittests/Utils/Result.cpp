#include "vara/Utils/Result.h"

#include "gtest/gtest.h"

namespace test {

enum ErrorCode { ERROR };

TEST(Result, okSimple) {
  Result<ErrorCode, int> R(Ok(42));

  ASSERT_TRUE(R);
  EXPECT_EQ(*R, 42);
}

TEST(Result, okMove) {
  auto R =
      Result<ErrorCode, std::unique_ptr<int>>(*Ok(std::make_unique<int>(42)));

  ASSERT_TRUE(R);
  EXPECT_EQ(**R, 42);
}

TEST(Result, okForward) {
  EXPECT_EQ(**(Result<ErrorCode, std::unique_ptr<int>>(
                Ok(std::make_unique<int>(42)))),
            42);
}

TEST(Result, errorSimple) {
  Result<ErrorCode, std::unique_ptr<int>> R(Error(ERROR));

  ASSERT_FALSE(R);
  EXPECT_EQ(R.getError(), ERROR);
}

TEST(Result, errorDefault) {
  Result<ErrorCode> R(Error(ERROR));

  ASSERT_FALSE(R);
  EXPECT_EQ(R.getError(), ERROR);
}

TEST(Result, errorMove) {
  auto R = Result<std::unique_ptr<ErrorCode>>(
      *Error(std::make_unique<ErrorCode>(ERROR)));

  ASSERT_FALSE(R);
  EXPECT_EQ(*R.getError(), ERROR);
}

TEST(Result, errorForward) {
  EXPECT_EQ(*(Result<std::unique_ptr<ErrorCode>>(
                  Error(std::make_unique<ErrorCode>(ERROR))))
                 .getError(),
            ERROR);
}

} // namespace test
