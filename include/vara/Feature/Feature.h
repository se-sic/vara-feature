#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/Location.h"
#include "vara/Feature/Relationship.h"

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <variant>

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
  using relationship_iterator = RelationshipListTy ::iterator;

private:
  std::string Name;
  llvm::SetVector<llvm::Value *> Values;
  size_t ID;
  bool Opt;
  std::optional<Location> Loc;
  llvm::SmallVector<Feature *, 1> Parents;
  FeatureListType Children;
  FeatureListType Excludes;
  FeatureListType Implications;
  FeatureListType Alternatives;
  RelationshipListTy Relationships;

protected:
  Feature(std::string Name, bool Opt, std::optional<Location> Loc)
      : Name(std::move(Name)), ID(std::hash<std::string>{}(Name)), Opt(Opt),
        Loc(std::move(Loc)) {}

public:
  Feature(const std::string &Name, llvm::Value *Val)
      : Name(Name), ID(std::hash<std::string>{}(Name)), Opt(false) {
    Values.insert(Val);
  }

  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] std::string getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] bool isRoot() const { return Parents.empty(); }

  void print(llvm::raw_ostream &OS) const {
    OS << getName() << " (";
    for (auto *Val : Values) {
      OS << Val << " ";
    }
    OS << ")"
       << " ID: " << getID();
  }

  LLVM_DUMP_METHOD
  void dump() const {
    print(llvm::outs());
    llvm::outs() << '\n';
  }

  feature_iterator begin() { return Children.begin(); }
  feature_iterator end() { return Children.end(); }
  llvm::iterator_range<feature_iterator> children() {
    return llvm::make_range(begin(), end());
  }
  void addChild(Feature *Child) { Children.push_back(Child); }

  llvm::SmallVector<Feature *, 1>::iterator parents_begin() {
    return Parents.begin();
  }
  llvm::SmallVector<Feature *, 1>::iterator parents_end() {
    return Parents.end();
  }
  llvm::iterator_range<llvm::SmallVector<Feature *, 1>::iterator> parents() {
    return llvm::make_range(parents_begin(), parents_end());
  }
  void addParent(Feature *Parent) { Parents.push_back(Parent); }

  feature_iterator excludes_begin() { return Excludes.begin(); }
  feature_iterator excludes_end() { return Excludes.end(); }
  llvm::iterator_range<feature_iterator> excludes() {
    return llvm::make_range(excludes_begin(), excludes_end());
  }
  void addExclude(Feature *Exclude) { Excludes.push_back(Exclude); }

  feature_iterator implications_begin() { return Implications.begin(); }
  feature_iterator implications_end() { return Implications.end(); }
  llvm::iterator_range<feature_iterator> implications() {
    return llvm::make_range(implications_begin(), implications_end());
  }
  void addImplication(Feature *Implication) {
    Implications.push_back(Implication);
  }

  feature_iterator alternatives_begin() { return Alternatives.begin(); }
  feature_iterator alternatives_end() { return Alternatives.end(); }
  llvm::iterator_range<feature_iterator> alternatives() {
    return llvm::make_range(alternatives_begin(), alternatives_end());
  }
  void addAlternative(Feature *Alternative) {
    Alternatives.push_back(Alternative);
  }

  relationship_iterator relationships_begin() { return Relationships.begin(); }
  relationship_iterator relationships_end() { return Relationships.end(); }
  llvm::iterator_range<relationship_iterator> relationships() {
    return llvm::make_range(relationships_begin(), relationships_end());
  }
  void addRelationship(std::unique_ptr<Relationship<Feature>> Relationship) {
    Relationships.push_back(std::move(Relationship));
  }

  [[nodiscard]] std::optional<Location> getLocation() const { return Loc; }

  [[nodiscard]] virtual std::string toString() const;

  using value_iterator = llvm::SetVector<llvm::Value *>::iterator;
  using const_value_iterator = llvm::SetVector<llvm::Value *>::const_iterator;

  inline void addValue(llvm::Value *Val) { Values.insert(Val); }

  value_iterator values_begin() { return Values.begin(); }
  [[nodiscard]] const_value_iterator values_begin() const {
    return Values.begin();
  }

  value_iterator values_end() { return Values.end(); }
  [[nodiscard]] const_value_iterator values_end() const { return Values.end(); }

  llvm::iterator_range<value_iterator> values() {
    return make_range(values_begin(), values_end());
  }
  [[nodiscard]] llvm::iterator_range<const_value_iterator> values() const {
    return make_range(values_begin(), values_end());
  }

  inline bool hasVal(llvm::Value *Val) { return Values.count(Val); }

  [[nodiscard]] inline std::size_t getID() const { return ID; }

  bool operator==(const Feature &Other) const { return Name == Other.Name; }

  bool operator!=(const Feature &Other) const { return Name != Other.Name; }
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(std::string Name, bool Opt,
                std::optional<Location> Loc = std::nullopt)
      : Feature(std::move(Name), Opt, std::move(Loc)) {}

  [[nodiscard]] std::string toString() const override;
};

/// Options with numeric values.
class NumericFeature : public Feature {
public:
  using ValuesVariantType =
      typename std::variant<std::pair<int, int>, std::vector<int>>;

private:
  ValuesVariantType V;

public:
  NumericFeature(std::string Name, bool Opt, ValuesVariantType V,
                 std::optional<Location> Loc = std::nullopt)
      : Feature(std::move(Name), Opt, std::move(Loc)), V(std::move(V)) {}

  ValuesVariantType getVals() { return V; }

  [[nodiscard]] std::string toString() const override;
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
