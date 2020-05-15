#include "vara/Feature/FeatureModelParser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

std::string TestDoc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                      "<!DOCTYPE vm SYSTEM \"vm.dtd\">\n"
                      "<vm name=\"Test\">\n"
                      "  <binaryOptions>\n"
                      "    <configurationOption>\n"
                      "      <name>root</name>\n"
                      "      <optional>False</optional>\n"
                      "    </configurationOption>\n"
                      "    <configurationOption>\n"
                      "      <name>A</name>\n"
                      "      <parent>root</parent>\n"
                      "      <optional>False</optional>\n"
                      "      <location>\n"
                      "        <path>main.c</path>\n"
                      "        <start>\n"
                      "          <line>6</line>\n"
                      "          <column>3</column>\n"
                      "        </start>\n"
                      "        <end>\n"
                      "          <line>6</line>\n"
                      "          <column>26</column>\n"
                      "        </end>\n"
                      "      </location>\n"
                      "    </configurationOption>\n"
                      "    <configurationOption>\n"
                      "      <name>C</name>\n"
                      "      <parent>B</parent>\n"
                      "      <optional>False</optional>\n"
                      "    </configurationOption>\n"
                      "  </binaryOptions>\n"
                      "  <numericOptions>\n"
                      "    <configurationOption>\n"
                      "      <name>B</name>\n"
                      "      <parent>root</parent>\n"
                      "      <optional>True</optional>\n"
                      "      <values>1;5;9</values>\n"
                      "    </configurationOption>\n"
                      "  </numericOptions>\n"
                      "  <booleanConstraints>\n"
                      "    <constraint>A | B</constraint>\n"
                      "  </booleanConstraints>\n"
                      "</vm>";

namespace vara::feature {
TEST(FeatureModelXmlParser, VerifyFeatureModel) {
  EXPECT_TRUE(
      vara::feature::FeatureModelXmlParser(TestDoc).verifyFeatureModel());
}

TEST(FeatureModelXmlParser, BuildFeatureModel) {
  std::unique_ptr<vara::feature::FeatureModel> FM =
      vara::feature::FeatureModelXmlParser(TestDoc).buildFeatureModel();
  ASSERT_NE(FM, nullptr);
  EXPECT_EQ(FM->getName(), "Test");
  EXPECT_EQ(FM->getPath(), std::filesystem::current_path());
  switch (0)
  case 0:
  default:
    if (const ::testing::AssertionResult gtest_ar =
            (::testing::internal::EqHelper::Compare(
                "std::distance((*FM).begin(), (*FM).end())", "4",
                std::distance(FM->begin(), FM->end()), 4)))
      ;
    else
      ::testing::internal::AssertHelper(
          ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 64,
          gtest_ar.failure_message()) = ::testing::Message();
  ;
  for (auto F : *FM) {
    std::string Name = F->getName();
    F->dump();
    if (Name == "root") {
      EXPECT_FALSE(F->isOptional());
      for (auto C : F->children()) {
        ASSERT_THAT(C->getName(), testing::AnyOfArray({"A", "B"}));
      }

    } else if (Name == "A") {
      EXPECT_FALSE(F->isOptional());
      EXPECT_EQ(F->children().begin(), F->children().end());
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->parents()).begin(), "
                    "(F->parents()).end())",
                    "1",
                    std::distance((F->parents()).begin(), (F->parents()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 77,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"root"}));
      }
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->children()).begin(), "
                    "(F->children()).end())",
                    "0",
                    std::distance((F->children()).begin(),
                                  (F->children()).end()),
                    0)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 81,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->alternatives()).begin(), "
                    "(F->alternatives()).end())",
                    "1",
                    std::distance((F->alternatives()).begin(),
                                  (F->alternatives()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 82,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto A : F->alternatives()) {
        ASSERT_THAT(A->getName(), testing::AnyOfArray({"B"}));
      }

    } else if (Name == "B") {
      EXPECT_TRUE(F->isOptional());
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->parents()).begin(), "
                    "(F->parents()).end())",
                    "1",
                    std::distance((F->parents()).begin(), (F->parents()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 89,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"root"}));
      }
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->children()).begin(), "
                    "(F->children()).end())",
                    "1",
                    std::distance((F->children()).begin(),
                                  (F->children()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 93,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto C : F->children()) {
        ASSERT_THAT(C->getName(), testing::AnyOfArray({"C"}));
      }
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->alternatives()).begin(), "
                    "(F->alternatives()).end())",
                    "1",
                    std::distance((F->alternatives()).begin(),
                                  (F->alternatives()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 97,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto A : F->alternatives()) {
        ASSERT_THAT(A->getName(), testing::AnyOfArray({"A"}));
      }
      auto NF = dynamic_cast<NumericFeature *>(F);
      ASSERT_NE(NF, nullptr);
      EXPECT_TRUE(std::holds_alternative<std::vector<int>>(NF->getVals()));
      EXPECT_THAT(std::get<std::vector<int>>(NF->getVals()),
                  testing::ElementsAre(1, 5, 9));
    } else if (Name == "C") {
      EXPECT_FALSE(F->isOptional());
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->parents()).begin(), "
                    "(F->parents()).end())",
                    "1",
                    std::distance((F->parents()).begin(), (F->parents()).end()),
                    1)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 107,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      for (auto P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"B"}));
      }
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->children()).begin(), "
                    "(F->children()).end())",
                    "0",
                    std::distance((F->children()).begin(),
                                  (F->children()).end()),
                    0)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 111,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
      switch (0)
      case 0:
      default:
        if (const ::testing::AssertionResult gtest_ar =
                (::testing::internal::EqHelper::Compare(
                    "std::distance((F->alternatives()).begin(), "
                    "(F->alternatives()).end())",
                    "0",
                    std::distance((F->alternatives()).begin(),
                                  (F->alternatives()).end()),
                    0)))
          ;
        else
          ::testing::internal::AssertHelper(
              ::testing::TestPartResult::kNonFatalFailure, "_file_name_", 112,
              gtest_ar.failure_message()) = ::testing::Message();
      ;
    } else {
      FAIL();
    }
  }
}
} // namespace vara::feature
