#ifndef VARA_FEATURE_FEATURESOURCERANGE_H
#define VARA_FEATURE_FEATURESOURCERANGE_H

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

  // TODO(s9latimm): remove NOLINT
  FeatureSourceRange(
      fs::path Path,
      std::optional<FeatureSourceLocation> Start = std::nullopt, // NOLINT
      std::optional<FeatureSourceLocation> End = std::nullopt)   // NOLINT
      : Path(std::move(Path)), Start(Start), End(End) {}         // NOLINT

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
    return Path == Other.Path and Start == Other.Start and End == Other.End;
  }

  inline bool operator!=(const FeatureSourceRange &Other) const {
    return !(*this == Other);
  }

private:
  fs::path Path;
  std::optional<FeatureSourceLocation> Start;
  std::optional<FeatureSourceLocation> End;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURESOURCERANGE_H
