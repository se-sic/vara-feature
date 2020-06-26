#include "vara/Feature/Feature.h"

#include <iterator>

namespace vara::feature {

std::string Feature::toString() const {
  std::stringstream StrS;
  StrS << "  name: \"" << Name
       << "\",\n  optional: " << (Opt ? "true" : "false") << ",\n  ";
  if (Loc) {
    StrS << "location: " << Loc->toString() << ",\n  ";
  }
  if (Parent) {
    StrS << "parent: " << Parent->Name << ",\n  ";
  }
  StrS << "children: [";
  for (const auto &Child : Children) {
    StrS << Child->Name << ",";
  }
  StrS << "],\n  excludes: [";
  for (const auto &Exclude : Excludes) {
    StrS << Exclude->Name << ",";
  }
  StrS << "],\n  implies: [";
  for (const auto &Implication : Implications) {
    StrS << Implication->Name << ",";
  }
  StrS << "],\n  alternatives: [";
  for (const auto &Alternative : Alternatives) {
    StrS << Alternative->Name << ",";
  }
  StrS << "]";
  return StrS.str();
}

std::string NumericFeature::toString() const {
  std::stringstream StrS;
  StrS << Feature::toString();
  if (std::holds_alternative<std::pair<int, int>>(Values)) {
    std::pair<int, int> Pair = std::get<std::pair<int, int>>(Values);
    StrS << ",\n  minValue: " << Pair.first << ",\n  maxValue: " << Pair.second;
  } else {
    StrS << ",\n  values: [";
    for (const auto &Val : std::get<std::vector<int>>(Values)) {
      StrS << Val << ",";
    }
    StrS << "]";
  }
  return StrS.str();
}

std::string BinaryFeature::toString() const { return Feature::toString(); }

bool Feature::operator<(const vara::feature::Feature &F) const {
  if (*this == F || (this->isRoot() && F.isRoot())) { // not a tree
    return this->getName().lower() < F.getName().lower();
  }
  if (this->isRoot()) {
    return true;
  }
  if (F.isRoot()) {
    return false;
  }

  // Find roots of subtrees containing either A (this) or B (F)  which have a
  // common parent and compare them lexicographically.

  std::stack<const vara::feature::Feature *> TraceA;
  std::stack<const vara::feature::Feature *> TraceB;

  for (const auto *Head = this; Head; Head = Head->getParent()) {
    TraceA.push(Head); // path from A to root
  }
  for (const auto *Head = &F; Head; Head = Head->getParent()) {
    TraceB.push(Head); // path from B to root
  }

  assert(!TraceA.empty() && !TraceB.empty());
  while (!TraceA.empty() && !TraceB.empty() &&
         TraceA.top() == TraceB.top()) { // skip common ancestors
    TraceA.pop();
    TraceB.pop();
  }

  assert(!TraceA.empty() || !TraceB.empty());
  if (TraceA.empty()) { // B in subtree of A
    return true;
  }
  if (TraceB.empty()) { // A in subtree of B
    return false;
  }

  return TraceA.top()->getName().lower() < TraceB.top()->getName().lower();
}
} // namespace vara::feature
