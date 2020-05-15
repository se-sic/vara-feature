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
  ASSERT_NE(FM->getRoot(), nullptr);
  EXPECT_EQ(FM->getRoot()->getName(), "root");
  EXPECT_EQ(std::distance(FM->begin(), FM->end()), 4);
  for (auto *F : *FM) {
    std::string Name = F->getName();
    F->dump();
    if (Name == "root") {
      EXPECT_FALSE(F->isOptional());
      for (auto *C : F->children()) {
        ASSERT_THAT(C->getName(), testing::AnyOfArray({"A", "B"}));
      }
    } else if (Name == "A") {
      EXPECT_FALSE(F->isOptional());
      EXPECT_EQ(F->children().begin(), F->children().end());
      EXPECT_EQ(std::distance(F->parents().begin(), F->parents().end()), 1);
      for (auto *P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"root"}));
      }
      EXPECT_EQ(std::distance(F->children().begin(), F->children().end()), 0);
      EXPECT_EQ(
          std::distance(F->alternatives().begin(), F->alternatives().end()), 1);
      for (auto *A : F->alternatives()) {
        ASSERT_THAT(A->getName(), testing::AnyOfArray({"B"}));
      }
    } else if (Name == "B") {
      EXPECT_TRUE(F->isOptional());
      EXPECT_EQ(std::distance(F->parents().begin(), F->parents().end()), 1);
      for (auto *P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"root"}));
      }
      EXPECT_EQ(std::distance(F->children().begin(), F->children().end()), 1);
      for (auto *C : F->children()) {
        ASSERT_THAT(C->getName(), testing::AnyOfArray({"C"}));
      }
      EXPECT_EQ(
          std::distance(F->alternatives().begin(), F->alternatives().end()), 1);
      for (auto *A : F->alternatives()) {
        ASSERT_THAT(A->getName(), testing::AnyOfArray({"A"}));
      }
      auto *NF = dynamic_cast<NumericFeature *>(F);
      ASSERT_NE(NF, nullptr);
      EXPECT_TRUE(std::holds_alternative<std::vector<int>>(NF->getVals()));
      EXPECT_THAT(std::get<std::vector<int>>(NF->getVals()),
                  testing::ElementsAre(1, 5, 9));
    } else if (Name == "C") {
      EXPECT_FALSE(F->isOptional());
      EXPECT_EQ(std::distance(F->parents().begin(), F->parents().end()), 1);
      for (auto *P : F->parents()) {
        ASSERT_THAT(P->getName(), testing::AnyOfArray({"B"}));
      }
      EXPECT_EQ(std::distance(F->children().begin(), F->children().end()), 0);
      EXPECT_EQ(
          std::distance(F->alternatives().begin(), F->alternatives().end()), 0);
    } else {
      FAIL();
    }
  }
}
} // namespace vara::feature
