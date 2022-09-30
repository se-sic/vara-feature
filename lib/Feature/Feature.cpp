#include "vara/Feature/Feature.h"

#include <iterator>

namespace vara::feature {

std::string Feature::toString() const {
  std::stringstream StrS;
  StrS << "  name: \"" << Name
       << "\",\n  optional: " << (Opt ? "true" : "false") << ",\n  ";
  if (hasLocations()) {
    StrS << "locations: ";
    std::for_each(Locations.begin(), Locations.end(),
                  [&StrS](const FeatureSourceRange &Fsr) {
                    StrS << llvm::formatv("[{0}]", Fsr.toString()).str();
                  });
    StrS << ",\n  ";
  }
  if (getParentFeature()) {
    StrS << "parent: " << getParentFeature()->getName().str() << ",\n  ";
  }
  StrS << "children: [";
  for (const auto &Child : children()) {
    if (auto *F = llvm::dyn_cast<Feature>(Child); F) {
      StrS << F->getName().str() << ",";
    }
  }
  StrS << "]";
  return StrS.str();
}

std::string NumericFeature::toString() const {
  std::stringstream StrS;
  StrS << Feature::toString();
  if (std::holds_alternative<ValueRangeType>(Values)) {
    auto Pair = std::get<ValueRangeType>(Values);
    StrS << ",\n  minValue: " << Pair.first << ",\n  maxValue: " << Pair.second;
  } else {
    StrS << ",\n  values: [";
    for (const auto &Val : std::get<ValueListType>(Values)) {
      StrS << Val << ",";
    }
    StrS << "]";
  }
  return StrS.str();
}

std::string BinaryFeature::toString() const { return Feature::toString(); }

/// Find roots of subtrees containing either A (this) or B (Other) which have a
/// common parent feature and compare them lexicographically. If no such node
/// can be found compare names directly.
bool Feature::operator<(const Feature &Other) const {
  std::stack<const Feature *> TraceA;
  std::stack<const Feature *> TraceB;

  for (const auto *Head = this; Head; Head = Head->getParentFeature()) {
    TraceA.push(Head); // path from A to root
  }
  for (const auto *Head = &Other; Head; Head = Head->getParentFeature()) {
    TraceB.push(Head); // path from B to root
  }
  assert(!TraceA.empty() && !TraceB.empty());

  if (TraceA.top() != TraceB.top()) { // different roots
    return this->getName().lower() < Other.getName().lower();
  }
  while (!TraceA.empty() && !TraceB.empty() &&
         TraceA.top() == TraceB.top()) { // skip common ancestors
    TraceA.pop();
    TraceB.pop();
  }

  if (TraceA.empty()) {     // B in subtree of A
    return !TraceB.empty(); // B not equal A
  }
  if (TraceB.empty()) { // A in subtree of B
    return false;
  }
  return TraceA.top()->getName().lower() < TraceB.top()->getName().lower();
}
} // namespace vara::feature
