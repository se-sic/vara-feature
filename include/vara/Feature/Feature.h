#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/FeatureSourceRange.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Support/raw_ostream.h"

#include <stack>
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

  enum FeatureType { BINARY, NUMERIC };

  Feature(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] FeatureType getType() const { return T; }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

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

  bool isParent() { return Parent != nullptr; }
  [[nodiscard]] Feature *getParent() const { return Parent; }

  Feature &operator=(const Feature &) = delete;

  bool operator==(const vara::feature::Feature &F) const {
    return getName().lower() == F.getName().lower();
  }

  bool operator<(const vara::feature::Feature &F) const;

  bool operator>(const vara::feature::Feature &F) const {
    return F.operator<(*this);
  }

  [[nodiscard]] FeatureSourceRange *getFeatureSourceRange() {
    return Loc.has_value() ? &Loc.value() : nullptr;
  }

  //===--------------------------------------------------------------------===//
  // Utility
  [[nodiscard]] virtual std::string toString() const;

  void print(std::ostream &Out) const { Out << toString() << std::endl; }
  void print(llvm::raw_ostream &Out) const { Out << toString() << '\n'; }

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << "\n"; }

protected:
  Feature(FeatureType T, string Name, bool Opt,
          std::optional<FeatureSourceRange> Loc, Feature *Parent,
          FeatureSetType Children, FeatureSetType Excludes,
          FeatureSetType Implications, FeatureSetType Alternatives)
      : T(T), Name(std::move(Name)), Opt(Opt), Loc(std::move(Loc)),
        Parent(Parent), Children(std::move(Children)),
        Excludes(std::move(Excludes)), Implications(std::move(Implications)),
        Alternatives(std::move(Alternatives)) {}

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  FeatureType T;
  string Name;
  bool Opt;
  std::optional<FeatureSourceRange> Loc;
  Feature *Parent;
  FeatureSetType Children;
  FeatureSetType Excludes;
  FeatureSetType Implications;
  FeatureSetType Alternatives;

  void addChild(Feature *F) { Children.insert(F); }

  void setParent(Feature *F) { Parent = F; }

  void addExclude(Feature *F) { Excludes.insert(F); }

  void addAlternative(Feature *F) { Alternatives.insert(F); }

  void addImplication(Feature *F) { Implications.insert(F); }
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(string Name, bool Opt = false,
                std::optional<FeatureSourceRange> Loc = std::nullopt,
                Feature *Parent = nullptr, FeatureSetType Children = {},
                FeatureSetType Excludes = {}, FeatureSetType Implications = {},
                FeatureSetType Alternatives = {})
      : Feature(BINARY, std::move(Name), Opt, std::move(Loc), Parent,
                std::move(Children), std::move(Excludes),
                std::move(Implications), std::move(Alternatives)) {}

  [[nodiscard]] string toString() const override;

  static bool classof(const Feature *F) { return F->getType() == BINARY; }
};

/// Options with numeric values.
class NumericFeature : public Feature {
public:
  using ValuesVariantType =
      typename std::variant<std::pair<int, int>, std::vector<int>>;

  NumericFeature(string Name, ValuesVariantType Values, bool Opt = false,
                 std::optional<FeatureSourceRange> Loc = std::nullopt,
                 Feature *Parent = nullptr, FeatureSetType Children = {},
                 FeatureSetType Excludes = {}, FeatureSetType Implications = {},
                 FeatureSetType Alternatives = {})
      : Feature(NUMERIC, std::move(Name), Opt, std::move(Loc), Parent,
                std::move(Children), std::move(Excludes),
                std::move(Implications), std::move(Alternatives)),
        Values(std::move(Values)) {}

  [[nodiscard]] ValuesVariantType getValues() const { return Values; }

  [[nodiscard]] string toString() const override;

  static bool classof(const Feature *F) { return F->getType() == NUMERIC; }

private:
  ValuesVariantType Values;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
