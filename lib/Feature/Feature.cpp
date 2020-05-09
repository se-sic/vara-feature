#include "vara/Feature/Feature.h"

namespace vara::feature {

std::string Feature::toString() const {
  std::stringstream StrS;
  StrS << "name: \"" << Name << "\",\n  optional: " << (Opt ? "true" : "false")
       << ",\n  ";
  if(Loc) {
    StrS << "location: " <<  Loc->get()->toString() << ",\n  ";
  }
  StrS << "parents: [";
  for (const auto &Parent : Parents) {
    StrS << Parent->Name;
    if (Parent != Parents.back()) {
      StrS << ", ";
    }
  }
  StrS << "],\n  children: [";
  for (const auto &Child : Children) {
    StrS << Child->Name;
    if (Child != Children.back()) {
      StrS << ", ";
    }
  }
  StrS << "],\n  excludes: [";
  for (const auto &Exclude : Excludes) {
    StrS << Exclude->Name;
    if (Exclude != Excludes.back()) {
      StrS << ", ";
    }
  }
  StrS << "],\n  implies: [";
  for (const auto &Implication : Implications) {
    StrS << Implication->Name;
    if (Implication != Implications.back()) {
      StrS << ", ";
    }
  }
  StrS << "],\n  alternatives: [";
  for (const auto &Alternative : Alternatives) {
    StrS << Alternative->Name;
    if (Alternative != Alternatives.back()) {
      StrS << ", ";
    }
  }
  StrS << "],\n  relationships: [";
  for (const auto &Relationship : Relationships) {
    StrS << "\n    " << Relationship->toString();
    if (Relationship == Relationships.back()) {
      StrS << "\n  ";
    } else {
      StrS << ",";
    }
  }
  StrS << "]";
  return StrS.str();
}

std::string NumericFeature::toString() const {
  std::stringstream StrS;
  StrS << Feature::toString();
  if (std::holds_alternative<std::pair<int, int>>(Vals)) {
    std::pair<int, int> Pair = std::get<std::pair<int, int>>(Vals);
    StrS << ",\n  minValue: " << Pair.first << ",\n  maxValue: " << Pair.second;
  } else {
    StrS << ",\n  values: [";
    for (const auto &Val : std::get<std::vector<int>>(Vals)) {
      StrS << Val;
      if (Val != std::get<std::vector<int>>(Vals).back()) {
        StrS << ", ";
      }
    }
    StrS << "]";
  }
  return StrS.str();
}

std::string BinaryFeature::toString() const { return Feature::toString(); }

} // namespace vara::feature
