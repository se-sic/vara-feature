#include "vara/Utils/Result.h"

#include "gtest/gtest.h"

namespace test {

enum ErrorCode { ERROR };

TEST(Result, okSimple) {
  auto O = Ok(42);

  ASSERT_TRUE(O);
  EXPECT_EQ(*O, 42);
}

TEST(Result, okExtract) {
  auto O = Ok(std::make_unique<int>(42));

  ASSERT_TRUE(O);
  EXPECT_EQ(*O.extractValue(), 42);
}

TEST(Result, okInline) { EXPECT_EQ(**Ok(std::make_unique<int>(42)), 42); }

TEST(Result, okForward) {
  auto O = Ok<std::pair<int, double>>(42, 13.37);

  ASSERT_TRUE(O);
  EXPECT_EQ((*O).first, 42);
}

TEST(Result, errorSimple) {
  auto E = Error(42);

  ASSERT_FALSE(E);
  EXPECT_EQ(*E, 42);
}

TEST(Result, errorExtract) {
  auto E = Error(std::make_unique<ErrorCode>(ERROR));

  ASSERT_FALSE(E);
  EXPECT_EQ(*E.extractError(), ERROR);
}

TEST(Result, errorInline) {
  EXPECT_EQ(**Error(std::make_unique<ErrorCode>(ERROR)), ERROR);
}

TEST(Result, errorForward) {
  auto E = Error<std::pair<int, double>>(42, 13.37);

  ASSERT_FALSE(E);
  EXPECT_EQ((*E).first, 42);
}

TEST(Result, resultOkSimple) {
  auto R = Result<ErrorCode, int>(Ok(42));

  ASSERT_TRUE(R);
  EXPECT_EQ(*R, 42);
}

TEST(Result, resultOkMove) {
  auto O = Ok(std::make_unique<int>(42));

  auto R = Result<ErrorCode, std::unique_ptr<int>>(std::move(O));

  ASSERT_TRUE(R);
  EXPECT_EQ(*R.extractValue(), 42);
}

TEST(Result, resultOkInline) {
  EXPECT_EQ(**(Result<ErrorCode, std::unique_ptr<int>>(
                Ok(std::make_unique<int>(42)))),
            42);
}

TEST(Result, resultOkForward) {
  auto R = Result<ErrorCode, std::pair<int, double>>(42, 13.37);

  ASSERT_TRUE(R);
  EXPECT_EQ((*R).first, 42);
}

TEST(Result, resultErrorSimple) {
  auto R = Result<ErrorCode>(ERROR);

  ASSERT_FALSE(R);
  EXPECT_EQ(R.getError(), ERROR);
}

TEST(Result, resultErrorForward) {
  auto R = Result<std::pair<int, double>>(42, 13.37);

  ASSERT_FALSE(R);
  EXPECT_EQ(R.getError().first, 42);
}

TEST(Result, resultErrorMove) {
  auto E = Error(std::make_unique<ErrorCode>(ERROR));

  auto R = Result<std::unique_ptr<ErrorCode>>(std::move(E));

  ASSERT_FALSE(R);
  EXPECT_EQ(*R.extractError(), ERROR);
}

TEST(Result, resultErrorInline) {
  EXPECT_EQ(
      *Result<std::unique_ptr<ErrorCode>>(std::make_unique<ErrorCode>(ERROR))
           .getError(),
      ERROR);
}

TEST(Result, resultErrorRValue) {
  auto R = Result<std::unique_ptr<ErrorCode>>(
      Error(std::make_unique<ErrorCode>(ERROR)).extractError());

  ASSERT_FALSE(R);
  EXPECT_EQ(*R.extractError(), ERROR);
}

TEST(Result, copyResultError) {
  auto R1 = Result<ErrorCode, int *>(ERROR);

  ASSERT_FALSE(R1);
  auto R2 = Result<ErrorCode, void>(R1.getError());

  ASSERT_FALSE(R2);
  EXPECT_EQ(R2.getError(), ERROR);
}

} // namespace test
