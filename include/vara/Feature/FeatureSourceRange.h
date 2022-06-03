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
  public:
    static std::optional<FeatureMemberOffset>
    createFeatureMemberOffset(llvm::StringRef MemberOffset) {
      auto RSplit = MemberOffset.rsplit("::");
      if (RSplit.second.empty()) {
        // wrong format
        return std::nullopt;
      }
      llvm::SmallVector<llvm::StringRef, 1> Class;
      RSplit.first.split(Class, "::");
      return FeatureMemberOffset(Class, RSplit.second);
    }

    [[nodiscard]] std::string
    className(std::optional<size_t> Nested = std::nullopt) const {
      if (Nested.has_value()) {
        assert(Nested.value() < Class.size());
        return Class[Nested.value()];
      }
      std::stringstream StrS;
      StrS << Class[0];
      for (size_t Idx = 1; Idx < Class.size(); ++Idx) {
        StrS << "::" << Class[Idx];
      }
      return StrS.str();
    }

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
        : Member(Member.str()) {
      for (const auto C : Class) {
        this->Class.push_back(C.str());
      }
    }

    llvm::SmallVector<std::string, 1> Class;
    std::string Member;
  };

  FeatureSourceRange(
      fs::path Path, std::optional<FeatureSourceLocation> Start = std::nullopt,
      std::optional<FeatureSourceLocation> End = std::nullopt,
      Category CategoryKind = Category::necessary,
      std::optional<FeatureMemberOffset> MemberOffset = std::nullopt)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)),
        CategoryKind(CategoryKind), MemberOffset(std::move(MemberOffset)) {}

  FeatureSourceRange(
      fs::path Path, FeatureSourceLocation Start, FeatureSourceLocation End,
      Category CategoryKind = Category::necessary,
      std::optional<FeatureMemberOffset> MemberOffset = std::nullopt)
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

  [[nodiscard]] bool hasMemberOffset() const {
    return MemberOffset.has_value();
  }
  [[nodiscard]] FeatureMemberOffset *getMemberOffset() {
    return MemberOffset.has_value() ? &MemberOffset.value() : nullptr;
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
  std::optional<FeatureSourceLocation> Start;
  std::optional<FeatureSourceLocation> End;
  Category CategoryKind;
  std::optional<FeatureMemberOffset> MemberOffset;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURESOURCERANGE_H
