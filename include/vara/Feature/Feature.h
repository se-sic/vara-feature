#ifndef VARA_FEATUREMODEL_FMFEATURE_H
#define VARA_FEATUREMODEL_FMFEATURE_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"
#include <variant>

#include "vara/Feature/Relationship.h"

using std::string;
namespace vara {

//===----------------------------------------------------------------------===//
//                               Feature Class
//===----------------------------------------------------------------------===//

/// \brief Base class for components of \a FeatureModel.
class FMFeature {
public:
  using FeatureListType = typename llvm::SmallVector<FMFeature *, 3>;

  using RelationshipListTy =
      typename llvm::SmallVector<std::unique_ptr<Relationship<FMFeature>>, 3>;

  using feature_iterator = typename FeatureListType::iterator;

private:
  string Name;

  llvm::SmallVector<FMFeature *, 1> Parents;

  FeatureListType Children;

  FeatureListType Excludes;

  FeatureListType Implications;

  FeatureListType Alternatives;

  RelationshipListTy Relationships;

  bool Opt;

protected:
  FMFeature(string Name, bool Opt) : Name(std::move(Name)), Opt(Opt) {}

public:
  virtual ~FMFeature() = default;

  [[nodiscard]] string getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] bool isRoot() const { return Parents.empty(); }

  void print(std::ostream &Out) const { Out << toString() << std::endl; }
  void print(llvm::raw_ostream &Out) const { Out << toString() << '\n'; }

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << "\n"; }

  feature_iterator begin() { return Children.begin(); }
  feature_iterator end() { return Children.end(); }
  llvm::iterator_range<feature_iterator> children() {
    return llvm::make_range(begin(), end());
  }
  void addChild(FMFeature *Child) { Children.push_back(Child); }

  llvm::SmallVector<FMFeature *, 1>::iterator parents_begin() {
    return Parents.begin();
  }
  llvm::SmallVector<FMFeature *, 1>::iterator parents_end() {
    return Parents.end();
  }
  llvm::iterator_range<llvm::SmallVector<FMFeature *, 1>::iterator> parents() {
    return llvm::make_range(parents_begin(), parents_end());
  }
  void addParent(FMFeature *Parent) { Parents.push_back(Parent); }

  feature_iterator excludes_begin() { return Excludes.begin(); }
  feature_iterator excludes_end() { return Excludes.end(); }
  llvm::iterator_range<feature_iterator> excludes() {
    return llvm::make_range(excludes_begin(), excludes_end());
  }
  void addExclude(FMFeature *Exclude) { Excludes.push_back(Exclude); }

  feature_iterator implications_begin() { return Implications.begin(); }
  feature_iterator implications_end() { return Implications.end(); }
  llvm::iterator_range<feature_iterator> implications() {
    return llvm::make_range(implications_begin(), implications_end());
  }
  void addImplication(FMFeature *Implication) {
    Implications.push_back(Implication);
  }

  feature_iterator alternatives_begin() { return Alternatives.begin(); }
  feature_iterator alternatives_end() { return Alternatives.end(); }
  llvm::iterator_range<feature_iterator> alternatives() {
    return llvm::make_range(alternatives_begin(), alternatives_end());
  }
  void addAlternative(FMFeature *Alternative) {
    Alternatives.push_back(Alternative);
  }

  using relationship_iterator = RelationshipListTy ::iterator;

  relationship_iterator relationships_begin() { return Relationships.begin(); }
  relationship_iterator relationships_end() { return Relationships.end(); }
  llvm::iterator_range<relationship_iterator> relationships() {
    return llvm::make_range(relationships_begin(), relationships_end());
  }
  void addRelationship(std::unique_ptr<Relationship<FMFeature>> Relationship) {
    Relationships.push_back(std::move(Relationship));
  }

  [[nodiscard]] virtual std::string toString() const;
};

/// Options without arguments.
class BinaryFeature : public FMFeature {

public:
  BinaryFeature(string Name, bool Opt) : FMFeature(std::move(Name), Opt) {}

  [[nodiscard]] string toString() const override;
};

/// Options with numeric values.
class NumericFeature : public FMFeature {
private:
  std::variant<std::pair<int, int>, std::vector<int>> Vals;

public:
  NumericFeature(string Name, bool Opt,
                 std::variant<std::pair<int, int>, std::vector<int>> Vals)
      : FMFeature(std::move(Name), Opt), Vals(std::move(Vals)) {}

  [[nodiscard]] string toString() const override;
};

} // namespace vara

#endif // VARA_FEATUREMODEL_FEATUREMODEL_H
