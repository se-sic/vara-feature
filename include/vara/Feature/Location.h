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

class Location {
public:
  class LineColumnOffset : private std::pair<int, int> {
  public:
    LineColumnOffset(int Line, int Column)
        : std::pair<int, int>(Line, Column) {}

    [[nodiscard]] int getLineNumber() const { return this->first; }

    [[nodiscard]] int getColumnOffset() const { return this->second; }

    void setLineNumber(int Value) { this->first = Value; }

    void setColumnOffset(int Value) { this->second = Value; }

    [[nodiscard]] std::string toString() const {
      return llvm::formatv("{0}:{1}", getLineNumber(), getColumnOffset());
    }
  };

private:
  fs::path Path;
  std::optional<LineColumnOffset> Start;
  std::optional<LineColumnOffset> End;

public:
  Location(fs::path Path, std::optional<LineColumnOffset> Start,
           std::optional<LineColumnOffset> End)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)) {}

  [[nodiscard]] fs::path getPath() const { return Path; }

  [[nodiscard]] LineColumnOffset* getStart() {
    return Start.has_value() ? &Start.value() : nullptr;
  }

  [[nodiscard]] LineColumnOffset* getEnd() {
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
};

inline bool operator==(const Location::LineColumnOffset &This,
                       const Location::LineColumnOffset &Other) {
  return This.getLineNumber() == Other.getLineNumber() &&
         This.getColumnOffset() == Other.getColumnOffset();
}

} // namespace vara::feature

#endif // VARA_FEATURE_LOCATION_H
