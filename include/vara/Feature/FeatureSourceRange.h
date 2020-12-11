#ifndef VARA_FEATURE_FEATURESOURCERANGE_H
#define VARA_FEATURE_FEATURESOURCERANGE_H

#include "llvm/Support/FormatVariadic.h"

#ifdef STD_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
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
  enum Category {necessary, inessential };

  class FeatureSourceLocation {

  public:
    FeatureSourceLocation(int Line, int Column) : Line(Line), Column(Column) {}

    void setLineNumber(int LineNumber) { this->Line = LineNumber; }
    [[nodiscard]] int getLineNumber() const { return this->Line; }

    void setColumnOffset(int ColumnOffset) { this->Column = ColumnOffset; }
    [[nodiscard]] int getColumnOffset() const { return this->Column; }

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
    int Line;
    int Column;
  };

  FeatureSourceRange(fs::path Path,
                     std::optional<FeatureSourceLocation> Start = std::nullopt,
                     std::optional<FeatureSourceLocation> End = std::nullopt,
                     Category CategoryKind = Category::necessary)
      : Path(std::move(Path)), Start(Start), End(End), CategoryKind(CategoryKind) {}

  [[nodiscard]] Category getCategory() const { return this->CategoryKind; }
  void setCategory(Category Value) {
    this->CategoryKind = Value;
  }

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

  [[nodiscard]] std::string toString() const {
    std::stringstream StrS;
    StrS << Path.string();
    if (Start) {
      StrS << ":" << Start->toString();
    }
    if (End) {
      StrS << "-" << End->toString();
    }
    return StrS.str();
  }

  inline bool operator==(const FeatureSourceRange &Other) const {
    return CategoryKind == Other.CategoryKind and Path == Other.Path and Start == Other.Start and End == Other.End;
  }

  inline bool operator!=(const FeatureSourceRange &Other) const {
    return !(*this == Other);
  }

private:
  Category CategoryKind;
  fs::path Path;
  std::optional<FeatureSourceLocation> Start;
  std::optional<FeatureSourceLocation> End;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURESOURCERANGE_H
