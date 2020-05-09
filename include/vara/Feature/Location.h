#ifndef VARA_FEATURE_LOCATION_H
#define VARA_FEATURE_LOCATION_H

#include <optional>
#include <sstream>
#include <string>

namespace vara::feature {

class Location {
private:
  std::string Path;
  std::optional<std::pair<int, int>> Start;
  std::optional<std::pair<int, int>> End;

public:
  Location(std::string Path, std::optional<std::pair<int, int>> Start,
           std::optional<std::pair<int, int>> End)
      : Path(std::move(Path)), Start(std::move(Start)), End(std::move(End)) {}

  [[nodiscard]] std::string getPath() const { return Path; }

  [[nodiscard]] std::optional<std::pair<int, int>> getStart() const {
    return Start;
  }

  [[nodiscard]] std::optional<std::pair<int, int>> getEnd() const {
    return End;
  }

  std::string toString() {
    std::stringstream StrS;
    StrS << Path;
    if (Start) {
      StrS << ":" << (*Start).first << ":" << (*Start).second;
    }
    if (End) {
      StrS << "->" << (*End).first << ":" << (*End).second;
    }
    return StrS.str();
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_LOCATION_H
