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
    createFeatureMemberOffset(const std::string &MemberOffset) {
      auto FMemOff = FeatureMemberOffset(MemberOffset);
      if (FMemOff.Names.size() > 1 && !FMemOff.Names.back().empty()) {
        return FMemOff;
      }
      return std::nullopt;
    }

    [[nodiscard]] std::size_t depth() const { return Names.size() - 1; }

    llvm::StringRef operator[](std::size_t Index) const {
      assert(Index <= depth());
      return Names[Index];
    }

    inline bool operator==(const FeatureMemberOffset &Other) const {
      return Names == Other.Names;
    }

    inline bool operator!=(const FeatureMemberOffset &Other) const {
      return Names != Other.Names;
    }

    [[nodiscard]] std::string toString() const {
      std::stringstream StrS;
      StrS << Names[0];
      for (std::size_t Idx = 1; Idx < Names.size(); ++Idx) {
        StrS << "::" << Names[Idx];
      }
      return StrS.str();
    }

  private:
    FeatureMemberOffset(const std::string &MemberOffset) {
      llvm::SmallVector<llvm::StringRef, 2> Split;
      llvm::StringRef(MemberOffset).split(Split, "::");
      for (const auto Ref : Split) {
        Names.push_back(Ref.str());
      }
    }

    llvm::SmallVector<std::string, 2> Names;
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
