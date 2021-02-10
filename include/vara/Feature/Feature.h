#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/FeatureSourceRange.h"
#include "vara/Feature/FeatureTreeNode.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <stack>
#include <utility>
#include <variant>

using std::string;

namespace vara::feature {

namespace detail {
class FeatureModelModification;
} // namespace detail

//===----------------------------------------------------------------------===//
//                               Feature Class
//===----------------------------------------------------------------------===//

/// \brief Base class for components of \a FeatureModel.
class Feature : public FeatureTreeNode {
  friend class FeatureModel;
  friend class FeatureModelBuilder;
  friend class detail::FeatureModelModification;

public:
  enum class FeatureKind { FK_BINARY, FK_NUMERIC, FK_ROOT, FK_UNKNOWN };

  Feature(std::string Name)
      : FeatureTreeNode(NodeKind::NK_FEATURE), Kind(FeatureKind::FK_UNKNOWN),
        Name(std::move(Name)), Opt(false) {}
  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  Feature(Feature &&) = delete;
  Feature &operator=(Feature &&) = delete;
  ~Feature() override = default;

  [[nodiscard]] inline std::size_t hash() const {
    return std::hash<std::string>{}(getName().lower());
  }

  [[nodiscard]] FeatureKind getKind() const { return Kind; }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  /// Search parent feature in tree structure -- this may not exist or nullptr
  /// if node is already root.
  [[nodiscard]] Feature *getParentFeature() const {
    for (FeatureTreeNode *P = this->getParent(); P; P = P->getParent()) {
      auto *F = llvm::dyn_cast<Feature>(P);
      if (F) {
        return F;
      }
    }
    return nullptr;
  }

  //===--------------------------------------------------------------------===//
  // Operators

  /// Compare lowercase name assuming those are unique.
  bool operator==(const vara::feature::Feature &Other) const {
    // TODO(s9latimm): keys in FM are case sensitive
    return getName().lower() == Other.getName().lower();
  }
  bool operator!=(const vara::feature::Feature &Other) const {
    return !operator==(Other);
  }

  /// Compare in depth first ordering.
  bool operator<(const vara::feature::Feature &Other) const;
  bool operator>(const vara::feature::Feature &Other) const {
    return Other.operator<(*this);
  }

  //===--------------------------------------------------------------------===//
  // Locations
  [[nodiscard]] bool hasLocations() const { return !Locations.empty(); }

  void addLocation(FeatureSourceRange &Fsr) {
    Locations.push_back(std::move(Fsr));
  }
  std::vector<FeatureSourceRange>::iterator
  removeLocation(const FeatureSourceRange &Fsr) {
    return Locations.erase(std::find(Locations.begin(), Locations.end(), Fsr));
  }

  bool updateLocation(const FeatureSourceRange &OldFsr,
                      FeatureSourceRange NewFsr) {
    auto Loc = std::find(Locations.begin(), Locations.end(), OldFsr);
    if (Loc != Locations.end()) {
      *Loc = std::move(NewFsr);
      return true;
    }
    return false;
  }

  [[nodiscard]] std::vector<FeatureSourceRange>::iterator getLocationsBegin() {
    return Locations.begin();
  }
  [[nodiscard]] std::vector<FeatureSourceRange>::iterator getLocationsEnd() {
    return Locations.end();
  }
  using locations_iterator = typename std::vector<FeatureSourceRange>::iterator;
  [[nodiscard]] llvm::iterator_range<locations_iterator> getLocations() {
    return llvm::make_range(Locations.begin(), Locations.end());
  }

  //===--------------------------------------------------------------------===//
  // Constraints

  using constraint_iterator = typename std::vector<Constraint *>::iterator;
  using const_constraint_iterator =
      typename std::vector<Constraint *>::const_iterator;

  [[nodiscard]] llvm::iterator_range<constraint_iterator> constraints() {
    return llvm::make_range(Constraints.begin(), Constraints.end());
  }
  [[nodiscard]] llvm::iterator_range<const_constraint_iterator>
  constraints() const {
    return llvm::make_range(Constraints.begin(), Constraints.end());
  }

  //===--------------------------------------------------------------------===//
  // Excludes

  using excludes_iterator =
      typename std::vector<ExcludesConstraint *>::iterator;
  using const_excludes_iterator =
      typename std::vector<ExcludesConstraint *>::const_iterator;

  [[nodiscard]] llvm::iterator_range<excludes_iterator> excludes() {
    return llvm::make_range(Excludes.begin(), Excludes.end());
  }
  [[nodiscard]] llvm::iterator_range<const_excludes_iterator> excludes() const {
    return llvm::make_range(Excludes.begin(), Excludes.end());
  }

  //===--------------------------------------------------------------------===//
  // Implications

  using implications_iterator =
      typename std::vector<ImpliesConstraint *>::iterator;
  using const_implications_iterator =
      typename std::vector<ImpliesConstraint *>::const_iterator;

  [[nodiscard]] llvm::iterator_range<implications_iterator> implications() {
    return llvm::make_range(Implications.begin(), Implications.end());
  }
  [[nodiscard]] llvm::iterator_range<const_implications_iterator>
  implications() const {
    return llvm::make_range(Implications.begin(), Implications.end());
  }

  //===--------------------------------------------------------------------===//
  // Utility

  [[nodiscard]] virtual std::string toString() const;

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << '\n'; }

  static bool classof(const FeatureTreeNode *N) {
    return N->getKind() == NodeKind::NK_FEATURE;
  }

protected:
  Feature(FeatureKind Kind, string Name, bool Opt,
          std::vector<FeatureSourceRange> Locations,
          FeatureTreeNode *Parent = nullptr, const NodeSetType &Children = {})
      : FeatureTreeNode(NodeKind::NK_FEATURE, Parent, Children), Kind(Kind),
        Name(std::move(Name)), Locations(std::move(Locations)), Opt(Opt) {}

private:
  void addConstraint(Constraint *C) {
    Constraints.push_back(C);
    if (auto *I = llvm::dyn_cast<ImpliesConstraint>(C->getRoot()); I) {
      Implications.push_back(I);
    } else if (auto *E = llvm::dyn_cast<ExcludesConstraint>(C->getRoot()); E) {
      Excludes.push_back(E);
    }
  }

  void removeConstraintNonPreserve(Constraint *C) {
    Constraints.erase(std::remove(Constraints.begin(), Constraints.end(), C),
                      Constraints.end());
    if (auto *I = llvm::dyn_cast<ImpliesConstraint>(C->getRoot()); I) {
      Implications.erase(
          std::remove(Implications.begin(), Implications.end(), I),
          Implications.end());
    } else if (auto *E = llvm::dyn_cast<ExcludesConstraint>(C->getRoot()); E) {
      Excludes.erase(std::remove(Excludes.begin(), Excludes.end(), E),
                     Excludes.end());
    }
  }

  const FeatureKind Kind;
  string Name;
  std::vector<FeatureSourceRange> Locations;
  std::vector<Constraint *> Constraints;
  std::vector<ExcludesConstraint *> Excludes;
  std::vector<ImpliesConstraint *> Implications;
  bool Opt;
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(
      string Name, bool Opt = false,
      std::vector<FeatureSourceRange> Loc = std::vector<FeatureSourceRange>())
      : Feature(FeatureKind::FK_BINARY, std::move(Name), Opt, std::move(Loc)) {}

  [[nodiscard]] string toString() const override;

  static bool classof(const Feature *F) {
    return F->getKind() == FeatureKind::FK_BINARY;
  }
};

/// Options with numeric values.
class NumericFeature : public Feature {
public:
  using ValuesVariantType =
      typename std::variant<std::pair<int, int>, std::vector<int>>;

  NumericFeature(
      string Name, ValuesVariantType Values, bool Opt = false,
      std::vector<FeatureSourceRange> Loc = std::vector<FeatureSourceRange>())
      : Feature(FeatureKind::FK_NUMERIC, std::move(Name), Opt, std::move(Loc)),
        Values(std::move(Values)) {}

  [[nodiscard]] ValuesVariantType getValues() const { return Values; }

  [[nodiscard]] string toString() const override;

  static bool classof(const Feature *F) {
    return F->getKind() == FeatureKind::FK_NUMERIC;
  }

private:
  ValuesVariantType Values;
};

//===----------------------------------------------------------------------===//
//                          RootFeature Class
//===----------------------------------------------------------------------===//

class RootFeature : public Feature {
public:
  RootFeature(string Name = "root")
      : Feature(FeatureKind::FK_ROOT, std::move(Name), false, {}) {}

  static bool classof(const Feature *F) {
    return F->getKind() == FeatureKind::FK_ROOT;
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
