#include "vara/Feature/FeatureSourceRange.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureSourceLocation, comparison) {
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
          .value(),
      FeatureSourceRange::FeatureRevisionRange(
          "94fe792df46e64f438720295742b3b72c407cab6",
          "1ed40f72e772adaa3adfcc94b9f038e4f3382339"));

  EXPECT_EQ(L.getPath(), fs::current_path());
  EXPECT_EQ(L.getStart()->getLineNumber(), 1);
  EXPECT_EQ(L.getStart()->getColumnOffset(), 4);
  EXPECT_EQ(L.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(L.getEnd()->getColumnOffset(), 5);
  EXPECT_EQ(L.getCategory(), FeatureSourceRange::Category::inessential);
  ASSERT_TRUE(L.hasMemberOffset());
  EXPECT_EQ(L.getMemberOffset()->toString(), "class::memberOffset");
  ASSERT_TRUE(L.hasRevisionRange());
  EXPECT_EQ(L.revisionRange()->introducingCommit(),
            "94fe792df46e64f438720295742b3b72c407cab6");
  ASSERT_TRUE(L.revisionRange()->hasRemovingCommit());
  EXPECT_EQ(L.revisionRange()->removingCommit(),
            "1ed40f72e772adaa3adfcc94b9f038e4f3382339");
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

TEST(FeatureMemberOffset, testFormatMethod) {
  EXPECT_TRUE(FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat(
      "::member"));
  EXPECT_TRUE(FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat(
      "foo::member"));
  EXPECT_TRUE(FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat(
      "foo::bar::member"));

  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat("foo:bar"));
  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat("member"));
  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::isMemberOffsetFormat("foo::"));
}

TEST(FeatureMemberOffset, testFactoryMethod) {
  EXPECT_TRUE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "::member")
          .has_value());
  EXPECT_TRUE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo::member")
          .has_value());
  EXPECT_TRUE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo::bar::member")
          .has_value());

  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo:bar")
          .has_value());
  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "member")
          .has_value());
  EXPECT_FALSE(
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo::")
          .has_value());
}

TEST(FeatureMemberOffset, basicAccessor) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo::bar::member");

  EXPECT_EQ(Member->toString(), "foo::bar::member");
}

TEST(FeatureMemberOffset, individualAccessors) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "foo::bar::member");

  ASSERT_TRUE(Member.has_value());

  EXPECT_EQ(Member.value().className(), "foo::bar");
  EXPECT_EQ(Member.value().className(0), "bar");
  EXPECT_EQ(Member.value().className(1), "foo");
  EXPECT_EQ(Member.value().memberName(), "member");
}

TEST(FeatureMemberOffset, anonymous) {
  auto Member =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "::member");

  EXPECT_EQ(Member->toString(), "::member");
  EXPECT_EQ(Member.value().className(), "");
  EXPECT_EQ(Member.value().memberName(), "member");
}

TEST(FeatureMemberOffset, comparison) {
  auto Member1 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member1");
  auto Member2 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member2");
  auto Member3 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::member2");

  ASSERT_TRUE(Member1.has_value());
  ASSERT_TRUE(Member2.has_value());
  ASSERT_TRUE(Member3.has_value());

  EXPECT_NE(Member1.value(), Member2.value());
  EXPECT_NE(Member1.value(), Member3.value());
  EXPECT_EQ(Member2.value(), Member3.value());
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

TEST(FeatureSourceRange, comparison) {
  const auto MemOff1 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset1");
  ASSERT_TRUE(MemOff1.has_value());

  const auto MemOff2 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset2");
  ASSERT_TRUE(MemOff2.has_value());

  const auto MemOff3 =
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset2");
  ASSERT_TRUE(MemOff3.has_value());

  auto L1 = FeatureSourceRange(
      "path1", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(1, 20),
      FeatureSourceRange::Category::inessential, MemOff1.value());

  auto L2 = FeatureSourceRange(
      "path2", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(1, 20),
      FeatureSourceRange::Category::inessential, MemOff2.value());

  auto L3 = FeatureSourceRange(
      "path2", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(1, 20),
      FeatureSourceRange::Category::inessential, MemOff3.value());

  EXPECT_NE(L1, L2);
  EXPECT_NE(L1, L3);
  EXPECT_EQ(L2, L3);
}

TEST(FeatureSourceRange, clone) {
  auto FSR = std::make_unique<FeatureSourceRange>(
      "path", FeatureSourceRange::FeatureSourceLocation(1, 2),
      FeatureSourceRange::FeatureSourceLocation(3, 4),
      FeatureSourceRange::Category::inessential,
      FeatureSourceRange::FeatureMemberOffset::createFeatureMemberOffset(
          "class::memberOffset")
          .value());

  auto Clone = FeatureSourceRange(*FSR);
  FSR.reset();
  // NOLINTNEXTLINE
  EXPECT_DEATH(EXPECT_TRUE(FSR->hasStart()), ".*");

  EXPECT_EQ(Clone.getPath(), "path");
  EXPECT_EQ(Clone.getStart()->getLineNumber(), 1);
  EXPECT_EQ(Clone.getStart()->getColumnOffset(), 2);
  EXPECT_EQ(Clone.getEnd()->getLineNumber(), 3);
  EXPECT_EQ(Clone.getEnd()->getColumnOffset(), 4);
  EXPECT_EQ(Clone.getMemberOffset()->toString(), "class::memberOffset");
  EXPECT_EQ(Clone.getCategory(), FeatureSourceRange::Category::inessential);
}

} // namespace vara::feature
