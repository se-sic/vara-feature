#include "vara/Feature/Feature.h"
#include "vara/Feature/Constraint.h"

#include <iterator>

namespace vara::feature {

std::string Feature::toString() const {
  std::stringstream StrS;
  StrS << "  name: \"" << Name
       << "\",\n  optional: " << (Opt ? "true" : "false") << ",\n  ";
  if (Source) {
    StrS << "location: " << Source->toString() << ",\n  ";
  }
  if (getParentFeature()) {
    StrS << "parent: " << getParentFeature()->getName().str() << ",\n  ";
  }
  StrS << "children: [";
  for (const auto &Child : children()) {
    auto *F = llvm::dyn_cast<Feature>(Child);
    if (F) {
      StrS << F->getName().str() << ",";
    }
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

/// Find roots of subtrees containing either A (this) or B (Other) which have a
/// common parent feature and compare them lexicographically. If no such node
/// can be found compare names directly.
bool Feature::operator<(const vara::feature::Feature &Other) const {
  std::stack<const vara::feature::Feature *> TraceA;
  std::stack<const vara::feature::Feature *> TraceB;

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

llvm::iterator_range<Feature::excludes_iterator> Feature::excludes() {
  std::vector<const vara::feature::ExcludesConstraint *> Excludes;
  for (const auto *C : constraints()) {
    const auto *E = llvm::dyn_cast<vara::feature::ExcludesConstraint>(C);
    if (E) {
      Excludes.push_back(E);
    }
  }
  // TODO
  return llvm::make_range(nullptr, nullptr);
}

llvm::iterator_range<Feature::const_excludes_iterator>
Feature::excludes() const {
  std::vector<const vara::feature::ExcludesConstraint *> Excludes;
  for (const auto *C : constraints()) {
    const auto *E = llvm::dyn_cast<vara::feature::ExcludesConstraint>(C);
    if (E) {
      Excludes.push_back(E);
    }
  }
  // TODO
  return llvm::make_range(nullptr, nullptr);
}

bool Feature::hasExclude(const Feature *F) const {
  for (const auto *E : excludes()) {
    const auto *RHS =
        llvm::dyn_cast<PrimaryFeatureConstraint>(E->getRightOperand());
    const auto *LHS =
        llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand());
    if (RHS && LHS) {
      if (RHS->getFeature()->getName() == this->getName() &&
          LHS->getFeature()->getName() == F->getName()) {
        return true;
      }
    }
  }
  return false;
}
} // namespace vara::feature
