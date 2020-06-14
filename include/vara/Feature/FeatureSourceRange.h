#ifndef VARA_FEATURE_LOCATION_H
#define VARA_FEATURE_LOCATION_H

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

namespace vara::feature {

class FeatureSourceRange {
public:
  class FeatureSourceLocation : private std::pair<int, int> {
  public:
    FeatureSourceLocation(int Line, int Column)
        : std::pair<int, int>(Line, Column) {}

    [[nodiscard]] int getLineNumber() const { return this->first; }

    [[nodiscard]] int getColumnOffset() const { return this->second; }

    [[nodiscard]] std::string toString() const {
      return llvm::formatv("{0}:{1}", getLineNumber(), getColumnOffset());
    }
  };

private:
  fs::path Path;
  std::optional<FeatureSourceLocation> Start;
  std::optional<FeatureSourceLocation> End;

public:
  FeatureSourceRange(fs::path Path, std::optional<FeatureSourceLocation> Start,
                     std::optional<FeatureSourceLocation> End)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)) {}

  [[nodiscard]] fs::path getPath() const { return Path; }

  [[nodiscard]] std::optional<FeatureSourceLocation> getStart() const {
    return Start;
  }

  [[nodiscard]] std::optional<FeatureSourceLocation> getEnd() const {
    return End;
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
};

inline bool operator==(const FeatureSourceRange::FeatureSourceLocation &This,
                       const FeatureSourceRange::FeatureSourceLocation &Other) {
  return This.getLineNumber() == Other.getLineNumber() &&
         This.getColumnOffset() == Other.getColumnOffset();
}

inline bool operator<(const FeatureSourceRange::FeatureSourceLocation &This,
                      const FeatureSourceRange::FeatureSourceLocation &Other) {
  return This.getLineNumber() < Other.getLineNumber() ||
         (This.getLineNumber() == Other.getLineNumber() &&
          This.getColumnOffset() < Other.getColumnOffset());
}

inline bool operator>(const FeatureSourceRange::FeatureSourceLocation &This,
                      const FeatureSourceRange::FeatureSourceLocation &Other) {
  return operator<(Other, This);
}

} // namespace vara::feature

#endif // VARA_FEATURE_LOCATION_H
