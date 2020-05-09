#ifndef VARA_FEATURE_LOCATION_H
#define VARA_FEATURE_LOCATION_H

#include <optional>
#include <sstream>
#include <string>

namespace vara::feature {

class Location {
public:
  class TableEntry : private std::pair<int, int> {
  public:
    TableEntry(int Line, int Column) : std::pair<int, int>(Line, Column) {}

    [[nodiscard]] int getLine() const { return this->first; }

    [[nodiscard]] int getColumn() const { return this->second; }
  };

private:
  std::string Path;
  std::optional<TableEntry> Start;
  std::optional<TableEntry> End;

public:
  Location(std::string Path, std::optional<TableEntry> Start,
           std::optional<TableEntry> End)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)) {}

  [[nodiscard]] std::string getPath() const { return Path; }

  [[nodiscard]] std::optional<TableEntry> getStart() const { return Start; }

  [[nodiscard]] std::optional<TableEntry> getEnd() const { return End; }

  [[nodiscard]] std::string toString() const {
    std::stringstream StrS;
    StrS << Path;
    if (Start) {
      StrS << ":" << (*Start).getLine() << ":" << (*Start).getColumn();
    }
    if (End) {
      StrS << "->" << (*End).getLine() << ":" << (*End).getColumn();
    }
    return StrS.str();
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_LOCATION_H
