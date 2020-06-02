#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/FeatureSourceRange.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/raw_ostream.h"

#include <utility>
#include <variant>

using std::string;

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               Feature Class
//===----------------------------------------------------------------------===//

/// \brief Base class for components of \a FeatureModel.
class Feature {
public:
  using FeatureSetType = typename llvm::DenseSet<Feature *>;
  using feature_iterator = typename FeatureSetType::iterator;
  using const_feature_iterator = typename FeatureSetType::const_iterator;

private:
  unsigned int Index;
  string Name;
  std::optional<FeatureSourceRange> Loc;
  bool Opt;
  Feature *Parent;
  FeatureSetType Children;
  FeatureSetType Excludes;
  FeatureSetType Implications;
  FeatureSetType Alternatives;

protected:
  Feature(int Index, string Name, bool Opt,
          std::optional<FeatureSourceRange> Loc, Feature *Parent)
      : Name(std::move(Name)), Opt(Opt), Loc(std::move(Loc)), Parent(Parent),
        Index(Index) {}

public:
  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] virtual bool isBinary() const = 0;

  [[nodiscard]] virtual bool isNumeric() const = 0;

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

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
  bool isChild(Feature *PosChild) const {
    return std::find(children_begin(), children_end(), PosChild) != end();
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
  bool isAlternative(Feature *PosAlternative) const {
    return std::find(alternatives_begin(), alternatives_end(),
                     PosAlternative) != alternatives_end();
  }

  //===--------------------------------------------------------------------===//
  // Default
  feature_iterator begin() { return children_begin(); }
  feature_iterator end() { return children_end(); }
  [[nodiscard]] const_feature_iterator begin() const {
    return children_begin();
  }
  [[nodiscard]] const_feature_iterator end() const { return children_end(); }

  Feature *getParent() { return Parent; }

  [[nodiscard]] std::optional<FeatureSourceRange> getLocation() const {
    return Loc;
  }

  //===--------------------------------------------------------------------===//
  // Utility
  [[nodiscard]] virtual std::string toString() const;

  [[nodiscard]] unsigned int getIndex() const { return Index; }

  bool operator==(const Feature &F) const {
    return this->Index == F.getIndex() &&
           this->getName().lower() == F.getName().lower();
  }

  bool operator<(const Feature &F) const {
    return this->Index < F.getIndex() ||
           (this->Index == F.getIndex() &&
            this->getName().lower() < F.getName().lower());
  }

  bool operator>(const Feature &F) const { return F.operator<(*this); }

  class Builder;

protected:
  void setIndex(unsigned int Index) { this->Index = Index; }

  void addChild(Feature *Child) { Children.insert(Child); }

  void addExclude(Feature *Exclude) { Excludes.insert(Exclude); }

  void addAlternative(Feature *Alternative) {
    Alternatives.insert(Alternative);
  }

  void addImplication(Feature *Implication) {
    Implications.insert(Implication);
  }
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(string Name, bool Opt,
                std::optional<FeatureSourceRange> Loc = std::nullopt,
                Feature *Parent = nullptr, int Index = 0)
      : Feature(Index, std::move(Name), Opt, std::move(Loc), Parent) {}

  [[nodiscard]] bool isBinary() const override { return true; }

  [[nodiscard]] bool isNumeric() const override { return false; }

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
                 std::optional<FeatureSourceRange> Loc = std::nullopt,
                 Feature *Parent = nullptr, int Index = 0)
      : Feature(Index, std::move(Name), Opt, std::move(Loc), Parent),
        Values(Values) {}

  ValuesVariantType getVals() { return Values; }

  [[nodiscard]] bool isBinary() const override { return false; }

  [[nodiscard]] bool isNumeric() const override { return true; }

  [[nodiscard]] string toString() const override;
};

class Feature::Builder {
  std::unique_ptr<Feature> F;

public:
  Builder() : F(nullptr) {}

  Builder(string Name, bool Opt,
          std::optional<FeatureSourceRange> Loc = std::nullopt)
      : F(std::make_unique<BinaryFeature>(Name, Opt, Loc)) {}

  Builder(string Name, bool Opt, NumericFeature::ValuesVariantType Values,
          std::optional<FeatureSourceRange> Loc = std::nullopt)
      : F(std::make_unique<NumericFeature>(Name, Opt, Values, Loc)) {}

  Feature *get() { return F.get(); }

  void setIndex(unsigned int Index) { F->setIndex(Index); }

  void addChild(Feature *Child) { F->addChild(Child); }

  void addAlternative(Feature *Alternative) { F->addAlternative(Alternative); }

  void addExclude(Feature *Exclude) { F->addExclude(Exclude); }

  void addImplication(Feature *Implication) { F->addImplication(Implication); }

  std::unique_ptr<Feature> build() { return std::move(F); }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
