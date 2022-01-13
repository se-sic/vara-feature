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

#include <optional>
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
    FeatureMemberOffset(std::string MemberOffset)
        : MemberOffset(std::move(MemberOffset)) {}

  public:
    static llvm::Optional<FeatureMemberOffset>
    createFeatureMemberOffset(std::string MemberOffset) {
      // Member offsets need to have excaptly one :: separator
      if (isInMemberOffsetFormat(MemberOffset)) {
        return FeatureMemberOffset(std::move(MemberOffset));
      }

      return {};
    }

    static bool isInMemberOffsetFormat(llvm::StringRef PossibleMemberOffset) {
      return PossibleMemberOffset.contains("::") &&
             PossibleMemberOffset.count(':') == 2;
    }

    void setMemberOffset(std::string MemberOffset) {
      if (isInMemberOffsetFormat(MemberOffset)) {
        this->MemberOffset = std::move(MemberOffset);
      }
    }
    [[nodiscard]] std::string fullMemberOffset() const {
      return this->MemberOffset;
    }

    [[nodiscard]] llvm::StringRef className() const {
      llvm::StringRef TmpRef{MemberOffset};
      return TmpRef.substr(0, TmpRef.find_first_of(':'));
    }

    [[nodiscard]] llvm::StringRef memberName() const {
      llvm::StringRef TmpRef{MemberOffset};
      return TmpRef.substr(TmpRef.find_last_of(':') + 1);
    }

    [[nodiscard]] std::string toString() const { return fullMemberOffset(); }

    inline bool operator==(const FeatureMemberOffset &Other) const {
      return fullMemberOffset() == Other.fullMemberOffset();
    }

    inline bool operator!=(const FeatureMemberOffset &Other) const {
      return fullMemberOffset() != Other.fullMemberOffset();
    }

  private:
    std::string MemberOffset;
  };

  FeatureSourceRange(
      fs::path Path, std::optional<FeatureSourceLocation> Start = std::nullopt,
      std::optional<FeatureSourceLocation> End = std::nullopt,
      Category CategoryKind = Category::necessary,
      llvm::Optional<FeatureMemberOffset> MemberOffset = llvm::None)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)),
        CategoryKind(CategoryKind), MemberOffset(std::move(MemberOffset)) {}

  FeatureSourceRange(
      fs::path Path, FeatureSourceLocation Start, FeatureSourceLocation End,
      Category CategoryKind = Category::necessary,
      llvm::Optional<FeatureMemberOffset> MemberOffset = llvm::None)
      : FeatureSourceRange(std::move(Path), std::optional(std::move(Start)),
                           std::optional(std::move(End)), CategoryKind,
                           std::move(MemberOffset)) {}

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

  [[nodiscard]] bool hasStart() const { return Start.has_value(); }
  [[nodiscard]] FeatureSourceLocation *getStart() {
    return Start.has_value() ? &Start.value() : nullptr;
  }

  [[nodiscard]] bool hasEnd() const { return End.has_value(); }
  [[nodiscard]] FeatureSourceLocation *getEnd() {
    return End.has_value() ? &End.value() : nullptr;
  }

  [[nodiscard]] bool hasMemberOffset() const { return MemberOffset.hasValue(); }
  [[nodiscard]] FeatureMemberOffset *getMemberOffset() {
    return MemberOffset.hasValue() ? MemberOffset.getPointer() : nullptr;
  }

  [[nodiscard]] std::string toString() const {
    std::stringstream StrS;
    StrS << Path.string();
    if (Start) {
      StrS << ":" << Start->toString();
    }
    if (End) {
      StrS << "-" << End->toString();
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
  std::optional<FeatureSourceLocation> Start;
  std::optional<FeatureSourceLocation> End;
  Category CategoryKind;
  llvm::Optional<FeatureMemberOffset> MemberOffset;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURESOURCERANGE_H
