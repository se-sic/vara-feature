#ifndef VARA_FEATURE_FEATURESOURCERANGE_H
#define VARA_FEATURE_FEATURESOURCERANGE_H

#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/FormatVariadic.h"

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

#include <sstream>
#include <string>
#include <tuple>
#include <utility>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureSourceRange Class
//===----------------------------------------------------------------------===//

class FeatureSourceRange {
public:
  enum class Category { necessary, inessential };

  class FeatureRevisionRange {
  public:
    FeatureRevisionRange(std::string Introduced, std::string Removed)
        : Introduced(std::move(Introduced)), Removed(std::move(Removed)) {}
    FeatureRevisionRange(std::string Introduced)
        : FeatureRevisionRange(Introduced, std::move(Introduced)) {}

    [[nodiscard]] llvm::StringRef introduced() const { return Introduced; }

    [[nodiscard]] llvm::StringRef removed() const { return Removed; }

  private:
    std::string Introduced;
    std::string Removed;
  };

  class FeatureSourceLocation {

  public:
    FeatureSourceLocation(unsigned Line, unsigned Column)
        : Line(Line), Column(Column) {}
    FeatureSourceLocation(const FeatureSourceLocation &L) = default;
    FeatureSourceLocation &operator=(const FeatureSourceLocation &) = default;
    FeatureSourceLocation(FeatureSourceLocation &&) = default;
    FeatureSourceLocation &operator=(FeatureSourceLocation &&) = default;
    virtual ~FeatureSourceLocation() = default;

    void setLineNumber(unsigned LineNumber) { this->Line = LineNumber; }
    [[nodiscard]] unsigned getLineNumber() const { return this->Line; }

    void setColumnOffset(unsigned ColumnOffset) { this->Column = ColumnOffset; }
    [[nodiscard]] unsigned getColumnOffset() const { return this->Column; }

    [[nodiscard]] std::string toString() const {
      return llvm::formatv("{0}:{1}", getLineNumber(), getColumnOffset());
    }

    inline bool operator==(const FeatureSourceLocation &Other) const {
      return getLineNumber() == Other.getLineNumber() &&
             getColumnOffset() == Other.getColumnOffset();
    }

    inline bool operator<(const FeatureSourceLocation &Other) const {
      return getLineNumber() < Other.getLineNumber() ||
             (getLineNumber() == Other.getLineNumber() &&
              getColumnOffset() < Other.getColumnOffset());
    }

    inline bool operator>(const FeatureSourceLocation &Other) const {
      return Other.operator<(*this);
    }

  private:
    unsigned Line;
    unsigned Column;
  };

  class FeatureMemberOffset {
  public:
    [[nodiscard]] static llvm::Optional<FeatureMemberOffset>
    createFeatureMemberOffset(llvm::StringRef MemberOffset) {
      auto Split = splitMemberOffset(MemberOffset);
      if (!Split.hasValue()) {
        return llvm::None;
      }
      return FeatureMemberOffset(splitClass(Split.getValue().first),
                                 Split.getValue().second);
    }

    [[nodiscard]] static bool
    isMemberOffsetFormat(llvm::StringRef PossibleMemberOffset) {
      return splitMemberOffset(PossibleMemberOffset).hasValue();
    }

    [[nodiscard]] std::string
    className(llvm::Optional<size_t> Nested = llvm::None) const {
      if (Nested.hasValue()) {
        assert(Nested.getValue() < Class.size());
        return Class[Class.size() - Nested.getValue() - 1];
      }
      std::stringstream StrS;
      StrS << Class[0];
      for (size_t Idx = 1; Idx < Class.size(); ++Idx) {
        StrS << "::" << Class[Idx];
      }
      return StrS.str();
    }

    /// Get the number of nested classes.
    [[nodiscard]] size_t nestingDepth() const { return Class.size(); }

    [[nodiscard]] std::string memberName() const { return Member; }

    [[nodiscard]] std::string toString() const {
      return className() + "::" + memberName();
    }

    inline bool operator==(const FeatureMemberOffset &Other) const {
      return Member == Other.Member && Class == Other.Class;
    }

    inline bool operator!=(const FeatureMemberOffset &Other) const {
      return !(*this == Other);
    }

  private:
    FeatureMemberOffset(const llvm::SmallVector<llvm::StringRef, 1> &Class,
                        llvm::StringRef Member)
        : Class{Class.begin(), Class.end()}, Member(Member.str()) {}

    static llvm::Optional<std::pair<llvm::StringRef, llvm::StringRef>>
    splitMemberOffset(llvm::StringRef MemberOffset) {
      auto Split = MemberOffset.rsplit("::");
      if (Split.second.empty()) {
        // wrong format
        return llvm::None;
      }
      return Split;
    }

    static llvm::SmallVector<llvm::StringRef, 1>
    splitClass(llvm::StringRef Class) {
      llvm::SmallVector<llvm::StringRef, 1> Split;
      Class.split(Split, "::");
      return Split;
    }

    llvm::SmallVector<std::string, 1> Class;
    std::string Member;
  };

  FeatureSourceRange(
      fs::path Path, llvm::Optional<FeatureSourceLocation> Start = llvm::None,
      llvm::Optional<FeatureSourceLocation> End = llvm::None,
      Category CategoryKind = Category::necessary,
      llvm::Optional<FeatureMemberOffset> MemberOffset = llvm::None,
      llvm::Optional<FeatureRevisionRange> RevisionRange = llvm::None)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)),
        CategoryKind(CategoryKind), MemberOffset(std::move(MemberOffset)),
        RevisionRange(std::move(RevisionRange)) {}

  FeatureSourceRange(
      fs::path Path, FeatureSourceLocation Start, FeatureSourceLocation End,
      Category CategoryKind = Category::necessary,
      llvm::Optional<FeatureMemberOffset> MemberOffset = llvm::None,
      llvm::Optional<FeatureRevisionRange> RevisionRange = llvm::None)
      : FeatureSourceRange(std::move(Path), llvm::Optional(std::move(Start)),
                           llvm::Optional(std::move(End)), CategoryKind,
                           std::move(MemberOffset), std::move(RevisionRange)) {}

  FeatureSourceRange(const FeatureSourceRange &L) = default;
  FeatureSourceRange &operator=(const FeatureSourceRange &) = default;
  FeatureSourceRange(FeatureSourceRange &&) = default;
  FeatureSourceRange &operator=(FeatureSourceRange &&) = default;
  virtual ~FeatureSourceRange() = default;

  [[nodiscard]] Category getCategory() const { return this->CategoryKind; }
  void setCategory(Category Value) { this->CategoryKind = Value; }

  [[nodiscard]] fs::path getPath() const { return Path; }
  void setPath(const std::string &Value) {
    fs::path P(Value);
    this->Path = P;
  }

  [[nodiscard]] bool hasStart() const { return Start.hasValue(); }
  [[nodiscard]] FeatureSourceLocation *getStart() {
    return Start.hasValue() ? Start.getPointer() : nullptr;
  }

  [[nodiscard]] bool hasEnd() const { return End.hasValue(); }
  [[nodiscard]] FeatureSourceLocation *getEnd() {
    return End.hasValue() ? End.getPointer() : nullptr;
  }

  [[nodiscard]] bool hasMemberOffset() const { return MemberOffset.hasValue(); }
  [[nodiscard]] FeatureMemberOffset *getMemberOffset() {
    return MemberOffset.hasValue() ? MemberOffset.getPointer() : nullptr;
  }

  [[nodiscard]] bool hasRevisionRange() const {
    return RevisionRange.hasValue();
  }
  [[nodiscard]] FeatureRevisionRange *getRevisionRange() {
    return RevisionRange.hasValue() ? RevisionRange.getPointer() : nullptr;
  }

  [[nodiscard]] std::string toString() const {
    std::stringstream StrS;
    StrS << Path.string();
    if (Start) {
      StrS << ":" << Start->toString();
    }
    if (End) {
      StrS << "–" << End->toString();
    }
    if (MemberOffset) {
      StrS << " MemberOffset: " << MemberOffset->toString();
    }
    return StrS.str();
  }

  inline bool operator==(const FeatureSourceRange &Other) const {
    return CategoryKind == Other.CategoryKind and Path == Other.Path and
           Start == Other.Start and End == Other.End and
           MemberOffset == Other.MemberOffset;
  }

  inline bool operator!=(const FeatureSourceRange &Other) const {
    return !(*this == Other);
  }

private:
  fs::path Path;
  llvm::Optional<FeatureSourceLocation> Start;
  llvm::Optional<FeatureSourceLocation> End;
  Category CategoryKind;
  llvm::Optional<FeatureMemberOffset> MemberOffset;
  llvm::Optional<FeatureRevisionRange> RevisionRange;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURESOURCERANGE_H
