#include "vara/Solver/ConfigurationFactory.h"

#include "vara/Feature/ConstraintBuilder.h"
#include "vara/Feature/FeatureModelBuilder.h"

#include "Utils/UnittestHelper.h"
#include "gtest/gtest.h"

namespace vara::solver {

std::unique_ptr<feature::FeatureModel> getFeatureModel() {
  vara::feature::FeatureModelBuilder B;
  B.makeRoot("root");
  B.makeFeature<vara::feature::BinaryFeature>("Foo", true)
      ->addEdge("root", "Foo");
  B.makeFeature<vara::feature::BinaryFeature>("alt", false)
      ->addEdge("root", "alt");
  B.makeFeature<feature::BinaryFeature>("a", true)->addEdge("alt", "a");
  B.makeFeature<feature::BinaryFeature>("b", true)->addEdge("alt", "b");
  vara::feature::ConstraintBuilder CB;
  CB.feature("a").implies().lNot().feature("b");
  B.addConstraint(
      std::make_unique<vara::feature::FeatureModel::BooleanConstraint>(
          CB.build()));

  B.makeFeature<vara::feature::BinaryFeature>("A", false)->addEdge("root", "A");
  B.makeFeature<vara::feature::BinaryFeature>("A1", true)->addEdge("A", "A1");
  B.makeFeature<vara::feature::BinaryFeature>("A2", true)->addEdge("A", "A2");
  B.makeFeature<vara::feature::BinaryFeature>("A3", true)->addEdge("A", "A3");
  B.emplaceRelationship(
      vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE, "A");
  B.makeFeature<vara::feature::BinaryFeature>("B", false)->addEdge("root", "B");
  B.makeFeature<vara::feature::BinaryFeature>("B1", true)->addEdge("B", "B1");
  B.makeFeature<vara::feature::BinaryFeature>("B2", true)->addEdge("B", "B2");
  B.makeFeature<vara::feature::BinaryFeature>("B3", true)->addEdge("B", "B3");
  B.emplaceRelationship(
      vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE, "B");
  B.makeFeature<vara::feature::BinaryFeature>("C", false)->addEdge("root", "C");
  B.makeFeature<vara::feature::BinaryFeature>("C1", true)->addEdge("C", "C1");
  B.makeFeature<vara::feature::BinaryFeature>("C2", true)->addEdge("C", "C2");
  B.makeFeature<vara::feature::BinaryFeature>("C3", true)->addEdge("C", "C3");
  B.emplaceRelationship(vara::feature::Relationship::RelationshipKind::RK_OR,
                        "C");

  auto FM = B.buildFeatureModel();
  return FM;
}

TEST(ConfigurationFactory, GetAllConfigurations) {
  auto FM = getFeatureModel();
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FM);
  EXPECT_TRUE(ConfigResult);
  EXPECT_EQ(ConfigResult.extractValue().size(), 6 * 63);
}

TEST(ConfigurationFactory, GetAllConfigurations2) {
  auto FM = feature::loadFeatureModel(
      getTestResource("test_three_optional_features.xml"));
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FM);
  EXPECT_TRUE(ConfigResult);
  auto Configs = ConfigResult.extractValue();

  EXPECT_EQ(Configs.size(), 8);

  auto ConfigsStrings = std::vector<string>();
  for (auto &Config : Configs) {
    ConfigsStrings.push_back(Config->dumpToString());
  }

  auto UniqueConfigs =
      std::set<string>(ConfigsStrings.begin(), ConfigsStrings.end());
  EXPECT_EQ(Configs.size(), UniqueConfigs.size());
}

TEST(ConfigurationFactory, GetAllConfigurations3) {
  auto FM = feature::loadFeatureModel(getTestResource("test_msmr.xml"));
  auto ConfigResult = ConfigurationFactory::getAllConfigs(*FM);
  EXPECT_TRUE(ConfigResult);
  auto Configs = ConfigResult.extractValue();

  EXPECT_EQ(Configs.size(), 16);

  auto ConfigsStrings = std::vector<string>();
  for (auto &Config : Configs) {
    ConfigsStrings.push_back(Config->dumpToString());
  }

  auto UniqueConfigs =
      std::set<string>(ConfigsStrings.begin(), ConfigsStrings.end());
  EXPECT_EQ(Configs.size(), UniqueConfigs.size());
}

TEST(ConfigurationFactory, GetNConfigurations) {
  auto FM = getFeatureModel();
  auto ConfigResult = ConfigurationFactory::getNConfigs(*FM, 100);
  EXPECT_TRUE(ConfigResult);
  EXPECT_EQ(ConfigResult.extractValue().size(), 100);
}

TEST(ConfigurationFactory, IsValid) {
  auto FM = getFeatureModel();
  auto ConfigResult = ConfigurationFactory::isValid(*FM);
  EXPECT_TRUE(ConfigResult);
}

TEST(ConfigurationFactory, UseIterator) {
  auto FM = getFeatureModel();
  auto Iterator = ConfigurationFactory::getConfigIterator(*FM);
  int Count = 100;
  for (auto ConfigurationResult : Iterator) {
    EXPECT_TRUE(ConfigurationResult);
    Count--;
    if (Count == 0) {
      break;
    }
  }
}

} // namespace vara::solver
