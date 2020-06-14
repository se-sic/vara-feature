#include "vara/Feature/Feature.h"

#include <experimental/iterator>

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

bool Feature::FeatureDepthFirstComparator::operator()(
    vara::feature::Feature *A, vara::feature::Feature *B) const {
  std::stack<vara::feature::Feature *> TraceA;
  std::stack<vara::feature::Feature *> TraceB;
  if (A == B || A->isRoot()) {
    return true;
  }
  if (B->isRoot()) {
    return false;
  }
  for (vara::feature::Feature *Head = A; Head; Head = Head->getParent()) {
    TraceA.push(Head);
  }
  for (vara::feature::Feature *Head = B; Head; Head = Head->getParent()) {
    TraceB.push(Head);
  }
  assert(!TraceA.empty() && !TraceB.empty());
  while (!TraceA.empty() && !TraceB.empty() && TraceA.top() == TraceB.top()) {
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
