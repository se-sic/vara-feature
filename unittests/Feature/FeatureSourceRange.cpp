#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureSourceRange, comparison) {
  auto SelfLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  auto OtherLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(FeatureSourceRange, full) {
  auto L = FeatureSourceRange(
      fs::current_path(), FeatureSourceRange::FeatureSourceLocation(1, 4),
      FeatureSourceRange::FeatureSourceLocation(3, 5),
      FeatureSourceRange::Category::inessential,
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset")
          .getValue());

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
  EXPECT_EQ(L.getCategory(), FeatureSourceRange::Category::inessential);
  EXPECT_EQ(L.getMemberOffset()->toString().compare("class::memberOffset"), 0);
}

TEST(FeatureSourceRange, noMemberOffset) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4),
                              FeatureSourceRange::FeatureSourceLocation(3, 5),
                              FeatureSourceRange::Category::inessential);

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
  EXPECT_EQ(L.getCategory(), FeatureSourceRange::Category::inessential);
  EXPECT_EQ(L.getMemberOffset(), nullptr);
}

TEST(FeatureSourceLocation, basicAccessors) {
  auto TestLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(TestLCO.getLineNumber(), 3);
  EXPECT_EQ(TestLCO.getColumnOffset(), 4);
}

TEST(FeatureSourceLocation, basicSetter) {
  auto TestLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  TestLCO.setLineNumber(4);
  TestLCO.setColumnOffset(5);

  EXPECT_EQ(TestLCO.getLineNumber(), 4);
  EXPECT_EQ(TestLCO.getColumnOffset(), 5);
}

TEST(FeatureSourceLocation, comparison) {
  auto SelfLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);
  auto OtherLCO = FeatureSourceRange::FeatureSourceLocation(3, 4);

  EXPECT_EQ(SelfLCO, OtherLCO);
}

TEST(FeatureMemberOffset, offsetFormatCheck) {
  EXPECT_TRUE(FeatureSourceRange::FeatureMemberOffset::isInMemberOffsetFormat(
      "foo::bar"));
  EXPECT_TRUE(FeatureSourceRange::FeatureMemberOffset::isInMemberOffsetFormat(
      "class::member"));

  EXPECT_FALSE(FeatureSourceRange::FeatureMemberOffset::isInMemberOffsetFormat(
      "foo:bar"));
  EXPECT_FALSE(FeatureSourceRange::FeatureMemberOffset::isInMemberOffsetFormat(
      "classMember"));
  EXPECT_FALSE(FeatureSourceRange::FeatureMemberOffset::isInMemberOffsetFormat(
      "class::class::member"));
}

TEST(FeatureMemberOffset, testCreationFactoryMethod) {
  auto FMO = FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
      "class::member");

  EXPECT_TRUE(FMO);
  EXPECT_EQ(FMO.getValue().fullMemberOffset(), "class::member");

  EXPECT_EQ(FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
                "foo:bar"),
            llvm::None);
  EXPECT_EQ(FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
                "classMember"),
            llvm::None);
  EXPECT_EQ(FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
                "class::class::member"),
            llvm::None);
}

TEST(FeatureMemberOffset, basicAccessor) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member");

  EXPECT_EQ(Member->fullMemberOffset().compare("class::member"), 0);
}

TEST(FeatureMemberOffset, indivitualAccessors) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member");

  EXPECT_EQ(Member->className(), "class");
  EXPECT_EQ(Member->memberName(), "member");
}

TEST(FeatureMemberOffset, basicSetter) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member");
  Member->setMemberOffset("class::setMemberOffset");

  EXPECT_EQ(Member->fullMemberOffset(), "class::setMemberOffset");

  Member->setMemberOffset(
      "onlyMemberOffsetPart"); // don't override with wrong format
  EXPECT_EQ(Member->fullMemberOffset(), "class::setMemberOffset");
}

TEST(FeatureMemberOffset, comparison) {
  const std::string MemString1 = "class::member1";
  const std::string MemString2 = "class::member2";
  auto Member1 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          MemString1);
  auto Member2 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          MemString2);

  EXPECT_NE(Member1.getValue(), Member2.getValue());

  Member2->setMemberOffset(MemString1);

  EXPECT_EQ(Member1.getValue(), Member2.getValue());
}

TEST(FeatureSourceRange, basicAccessors) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4),
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
}

TEST(FeatureSourceRange, onlyStart) {
  auto L = FeatureSourceRange(fs::current_path(),
                              FeatureSourceRange::FeatureSourceLocation(1, 4));

  EXPECT_TRUE(L.hasStart());
  EXPECT_FALSE(L.hasEnd());
}

TEST(FeatureSourceRange, onlyEnd) {
  auto L = FeatureSourceRange(fs::current_path(), std::nullopt,
                              FeatureSourceRange::FeatureSourceLocation(3, 5));

  EXPECT_FALSE(L.hasStart());
  EXPECT_TRUE(L.hasEnd());
}

TEST(FeatureSourceRange, equality) {
  const auto *Path1 = "path1";
  const auto MemberOffset1 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset1");
  auto Fsl1start = FeatureSourceRange::FeatureSourceLocation(1, 2);
  auto Fsl1end = FeatureSourceRange::FeatureSourceLocation(1, 20);
  EXPECT_TRUE(MemberOffset1);
  auto L1 = FeatureSourceRange(Path1, Fsl1start, Fsl1end,
                               FeatureSourceRange::Category::inessential,
                               MemberOffset1.getValue());
  const auto *Path2 = "path2";
  const auto MemberOffset2 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset2");
  auto Fsl2start = FeatureSourceRange::FeatureSourceLocation(1, 2);
  auto Fsl2end = FeatureSourceRange::FeatureSourceLocation(1, 20);
  EXPECT_TRUE(MemberOffset2);
  auto L2 = FeatureSourceRange(Path2, Fsl2start, Fsl2end,
                               FeatureSourceRange::Category::inessential,
                               MemberOffset2.getValue());
  EXPECT_NE(L1, L2);

  L2.setPath(Path1);
  EXPECT_NE(L1, L2);

  EXPECT_TRUE(L2.hasMemberOffset());
  L2.getMemberOffset()->setMemberOffset("class::memberOffset1");
  EXPECT_EQ(L1, L2);
}

TEST(FeatureSourceRange, clone) {
  auto FSR = std::make_unique<FeatureSourceRange>(
      "path", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(3, 4),
      FeatureSourceRange::Category::inessential,
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset")
          .getValue());

  auto Clone = FeatureSourceRange(*FSR);
  FSR.reset();
  // NOLINTNEXTLINE
  EXPECT_DEATH(EXPECT_TRUE(FSR->hasStart()), ".*");

  EXPECT_EQ(Clone.getPath(), "path");
  EXPECT_EQ(Clone.getStart()->getLineNumber(), 1);
  EXPECT_EQ(Clone.getStart()->getColumnOffset(), 2);
  EXPECT_EQ(Clone.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(Clone.getEnd()->getColumnOffset(), 4);
  EXPECT_EQ(Clone.getMemberOffset()->toString().compare("class::memberOffset"),
            0);
  EXPECT_EQ(Clone.getCategory(), FeatureSourceRange::Category::inessential);
}

} // namespace vara::feature
