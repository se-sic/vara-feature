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
      FeatureSourceRange::FeatureMemberOffset("memberOffset"));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
  EXPECT_EQ(L.getCategory(), FeatureSourceRange::Category::inessential);
  EXPECT_EQ(L.getMemberOffset()->toString().compare("memberOffset"), 0);
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

TEST(FeatureMemberOffset, basicAccessor) {
  auto Member = FeatureSourceRange::FeatureMemberOffset("member");

  EXPECT_EQ(Member.getMemberOffset().compare("member"), 0);
}

TEST(FeatureMemberOffset, basicSetter) {
  auto Member = FeatureSourceRange::FeatureMemberOffset("member");
  Member.setMemberOffset("setMemberOffset");

  EXPECT_EQ(Member.getMemberOffset().compare("setMemberOffset"), 0);
}

TEST(FeatureMemberOffset, comparison) {
  const std::string MemString1 = "member1";
  const std::string MemString2 = "member2";
  auto Member1 = FeatureSourceRange::FeatureMemberOffset(MemString1);
  auto Member2 = FeatureSourceRange::FeatureMemberOffset(MemString2);

  EXPECT_NE(Member1, Member2);

  Member2.setMemberOffset(MemString1);

  EXPECT_EQ(Member1, Member2);
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
      FeatureSourceRange::FeatureMemberOffset("memberOffset1");
  auto Fsl1start = FeatureSourceRange::FeatureSourceLocation(1, 2);
  auto Fsl1end = FeatureSourceRange::FeatureSourceLocation(1, 20);
  auto L1 = FeatureSourceRange(Path1, Fsl1start, Fsl1end,
                               FeatureSourceRange::Category::inessential,
                               MemberOffset1);
  const auto *Path2 = "path2";
  const auto MemberOffset2 =
      FeatureSourceRange::FeatureMemberOffset("memberOffset2");
  auto Fsl2start = FeatureSourceRange::FeatureSourceLocation(1, 2);
  auto Fsl2end = FeatureSourceRange::FeatureSourceLocation(1, 20);
  auto L2 = FeatureSourceRange(Path2, Fsl2start, Fsl2end,
                               FeatureSourceRange::Category::inessential,
                               MemberOffset2);
  EXPECT_NE(L1, L2);

  L2.setPath(Path1);
  EXPECT_NE(L1, L2);

  L2.getMemberOffset()->setMemberOffset("memberOffset1");
  EXPECT_EQ(L1, L2);
}

TEST(FeatureSourceRange, clone) {
  auto FSR = std::make_unique<FeatureSourceRange>(
      "path", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(3, 4),
      FeatureSourceRange::Category::inessential,
      FeatureSourceRange::FeatureMemberOffset("memberOffset"));

  auto Clone = FeatureSourceRange(*FSR);
  FSR.reset();
  // NOLINTNEXTLINE
  EXPECT_DEATH(EXPECT_TRUE(FSR->hasStart()), ".*");

  EXPECT_EQ(Clone.getPath(), "path");
  EXPECT_EQ(Clone.getStart()->getLineNumber(), 1);
  EXPECT_EQ(Clone.getStart()->getColumnOffset(), 2);
  EXPECT_EQ(Clone.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(Clone.getEnd()->getColumnOffset(), 4);
  EXPECT_EQ(Clone.getMemberOffset()->toString().compare("memberOffset"), 0);
  EXPECT_EQ(Clone.getCategory(), FeatureSourceRange::Category::inessential);
}

} // namespace vara::feature
