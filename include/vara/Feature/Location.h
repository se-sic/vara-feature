#ifndef VARA_FEATURE_LOCATION_H
#define VARA_FEATURE_LOCATION_H

#include <filesystem> // TODO (s9latimm): remove experimental
#include <optional>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace vara::feature {

class Location {
public:
  class LineColumnOffset : private std::pair<int, int> {
  public:
    LineColumnOffset(int Line, int Column)
        : std::pair<int, int>(Line, Column) {}

    [[nodiscard]] int getLineNumber() const { return this->first; }

    [[nodiscard]] int getColumnOffset() const { return this->second; }
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

  [[nodiscard]] std::optional<LineColumnOffset> getStart() const {
    return Start;
  }

  [[nodiscard]] std::optional<LineColumnOffset> getEnd() const { return End; }

  [[nodiscard]] std::string toString() const {
    std::stringstream StrS;
    StrS << Path.string();
    if (Start) {
      StrS << ":" << (*Start).getLineNumber() << ":"
           << (*Start).getColumnOffset();
    }
    if (End) {
      StrS << "->" << (*End).getLineNumber() << ":" << (*End).getColumnOffset();
    }
    return StrS.str();
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_LOCATION_H
