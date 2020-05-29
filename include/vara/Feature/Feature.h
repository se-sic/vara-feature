#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/Location.h"
#include "vara/Feature/Relationship.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/raw_ostream.h"

#include <variant>

using std::string;

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               Feature Class
//===----------------------------------------------------------------------===//

/// \brief Base class for components of \a FeatureModel.
class Feature {
public:
  using FeatureListType = typename llvm::SmallVector<Feature *, 3>;
  using RelationshipListTy =
      typename llvm::SmallVector<std::unique_ptr<Relationship<Feature>>, 3>;
  using feature_iterator = typename FeatureListType::iterator;
  using const_feature_iterator = typename FeatureListType::const_iterator;
  using relationship_iterator = RelationshipListTy ::iterator;

private:
  string Name;
  std::optional<Location> Loc;
  bool Opt;
  llvm::SmallVector<Feature *, 1> Parents;
  FeatureListType Children;
  FeatureListType Excludes;
  FeatureListType Implications;
  FeatureListType Alternatives;
  RelationshipListTy Relationships;

protected:
  Feature(string Name, bool Opt, std::optional<Location> Loc)
      : Name(std::move(Name)), Opt(Opt), Loc(std::move(Loc)) {}

public:
  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] bool isRoot() const { return Parents.empty(); }

  void print(std::ostream &Out) const { Out << toString() << std::endl; }
  void print(llvm::raw_ostream &Out) const { Out << toString() << '\n'; }

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << "\n"; }

  //===--------------------------------------------------------------------===//
  // Children
  feature_iterator children_begin() { return Children.begin(); }
  feature_iterator children_end() { return Children.end(); }
  [[nodiscard]] const_feature_iterator children_begin() const {
    return Children.begin();
  }
  [[nodiscard]] const_feature_iterator children_end() const {
    return Children.end();
  }
  llvm::iterator_range<feature_iterator> children() {
    return llvm::make_range(children_begin(), children_end());
  }
  [[nodiscard]] llvm::iterator_range<const_feature_iterator> children() const {
    return llvm::make_range(children_begin(), children_end());
  }
  void addChild(Feature *Child) { Children.push_back(Child); }
  bool isChild(Feature *PosChild) const {
    return std::find(children_begin(), children_end(), PosChild) != end();
  }

  //===--------------------------------------------------------------------===//
  // Parents
  llvm::SmallVector<Feature *, 1>::iterator parents_begin() {
    return Parents.begin();
  }
  llvm::SmallVector<Feature *, 1>::iterator parents_end() {
    return Parents.end();
  }
  [[nodiscard]] llvm::SmallVector<Feature *, 1>::const_iterator
  parents_begin() const {
    return Parents.begin();
  }
  [[nodiscard]] llvm::SmallVector<Feature *, 1>::const_iterator
  parents_end() const {
    return Parents.end();
  }
  llvm::iterator_range<llvm::SmallVector<Feature *, 1>::iterator> parents() {
    return llvm::make_range(parents_begin(), parents_end());
  }
  [[nodiscard]] llvm::iterator_range<
      llvm::SmallVector<Feature *, 1>::const_iterator>
  parents() const {
    return llvm::make_range(parents_begin(), parents_end());
  }
  void addParent(Feature *Parent) { Parents.push_back(Parent); }
  [[nodiscard]] bool isParent(Feature *PosParent) const {
    return std::find(parents_begin(), parents_end(), PosParent) !=
           parents_end();
  }

  //===--------------------------------------------------------------------===//
  // Excludes
  feature_iterator excludes_begin() { return Excludes.begin(); }
  feature_iterator excludes_end() { return Excludes.end(); }
  [[nodiscard]] const_feature_iterator excludes_begin() const {
    return Excludes.begin();
  }
  [[nodiscard]] const_feature_iterator excludes_end() const {
    return Excludes.end();
  }
  llvm::iterator_range<feature_iterator> excludes() {
    return llvm::make_range(excludes_begin(), excludes_end());
  }
  [[nodiscard]] llvm::iterator_range<const_feature_iterator> excludes() const {
    return llvm::make_range(excludes_begin(), excludes_end());
  }
  void addExclude(Feature *Exclude) { Excludes.push_back(Exclude); }
  bool isExcluded(Feature *PosExclude) const {
    return std::find(excludes_begin(), excludes_end(), PosExclude) !=
           excludes_end();
  }

  //===--------------------------------------------------------------------===//
  // Implications
  feature_iterator implications_begin() { return Implications.begin(); }
  feature_iterator implications_end() { return Implications.end(); }
  [[nodiscard]] const_feature_iterator implications_begin() const {
    return Implications.begin();
  }
  [[nodiscard]] const_feature_iterator implications_end() const {
    return Implications.end();
  }
  llvm::iterator_range<feature_iterator> implications() {
    return llvm::make_range(implications_begin(), implications_end());
  }
  [[nodiscard]] llvm::iterator_range<const_feature_iterator>
  implications() const {
    return llvm::make_range(implications_begin(), implications_end());
  }
  void addImplication(Feature *Implication) {
    Implications.push_back(Implication);
  }
  bool implies(Feature *PosImplication) const {
    return std::find(implications_begin(), implications_end(),
                     PosImplication) != implications_end();
  }

  //===--------------------------------------------------------------------===//
  // Alternatives
  feature_iterator alternatives_begin() { return Alternatives.begin(); }
  feature_iterator alternatives_end() { return Alternatives.end(); }
  [[nodiscard]] const_feature_iterator alternatives_begin() const {
    return Alternatives.begin();
  }
  [[nodiscard]] const_feature_iterator alternatives_end() const {
    return Alternatives.end();
  }
  llvm::iterator_range<feature_iterator> alternatives() {
    return llvm::make_range(alternatives_begin(), alternatives_end());
  }
  [[nodiscard]] llvm::iterator_range<const_feature_iterator>
  alternatives() const {
    return llvm::make_range(alternatives_begin(), alternatives_end());
  }
  void addAlternative(Feature *Alternative) {
    Alternatives.push_back(Alternative);
  }
  bool isAlternative(Feature *PosAlternative) const {
    return std::find(alternatives_begin(), alternatives_end(),
                     PosAlternative) != alternatives_end();
  }

  //===--------------------------------------------------------------------===//
  // Relationships
  relationship_iterator relationships_begin() { return Relationships.begin(); }
  relationship_iterator relationships_end() { return Relationships.end(); }
  llvm::iterator_range<relationship_iterator> relationships() {
    return llvm::make_range(relationships_begin(), relationships_end());
  }
  void addRelationship(std::unique_ptr<Relationship<Feature>> Relationship) {
    Relationships.push_back(std::move(Relationship));
  }

  //===--------------------------------------------------------------------===//
  // Default
  feature_iterator begin() { return children_begin(); }
  feature_iterator end() { return children_end(); }
  [[nodiscard]] const_feature_iterator begin() const {
    return children_begin();
  }
  [[nodiscard]] const_feature_iterator end() const { return children_end(); }

  [[nodiscard]] std::optional<Location> getLocation() const { return Loc; }

  //===--------------------------------------------------------------------===//
  // Utility
  [[nodiscard]] virtual std::string toString() const;

  bool operator==(const Feature &F) const {
    return llvm::StringRef(this->getName()).lower() ==
           llvm::StringRef(F.getName()).lower();
  }

  bool operator<(const Feature &F) const {
    return llvm::StringRef(this->getName()).lower() <
           llvm::StringRef(F.getName()).lower();
  }
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(string Name, bool Opt,
                std::optional<Location> Loc = std::nullopt)
      : Feature(std::move(Name), Opt, std::move(Loc)) {}

  [[nodiscard]] string toString() const override;
};

/// Options with numeric values.
class NumericFeature : public Feature {
public:
  using ValuesVariantType =
      typename std::variant<std::pair<int, int>, std::vector<int>>;

private:
  ValuesVariantType Values;

public:
  NumericFeature(string Name, bool Opt, ValuesVariantType Values,
                 std::optional<Location> Loc = std::nullopt)
      : Feature(std::move(Name), Opt, std::move(Loc)),
        Values(std::move(Values)) {}

  ValuesVariantType getVals() { return Values; }

  [[nodiscard]] string toString() const override;
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
