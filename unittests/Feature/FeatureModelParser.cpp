#include "vara/Feature/FeatureModelParser.h"

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
TEST(FeatureModelXmlParser, verifyFeatureModel) {
  auto P = vara::feature::FeatureModelXmlParser(TestDoc);
  EXPECT_TRUE(P.verifyFeatureModel());
}

TEST(FeatureModelXmlParser, buildFeatureModel) {
  auto P = vara::feature::FeatureModelXmlParser(TestDoc);
  std::unique_ptr<vara::feature::FeatureModel> FM =
      vara::feature::FeatureModelXmlParser(TestDoc).buildFeatureModel();
  ASSERT_NE(FM, nullptr);
  EXPECT_EQ(FM.get()->getName(), "Test");
}
} // namespace vara::feature
