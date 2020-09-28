#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/FeatureSourceRange.h"

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

#include <llvm/ADT/SetVector.h>
#include <set>
#include <stack>
#include <utility>
#include <variant>

using std::string;

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               Feature Class
//===----------------------------------------------------------------------===//

/// \brief Base class for components of \a FeatureModel or \a FeatureRegion.
class Feature {
public:
  using FeatureSetType = typename std::set<Feature *>;
  using feature_iterator = typename FeatureSetType::iterator;
  using const_feature_iterator = typename FeatureSetType::const_iterator;

  enum class FeatureKind { FK_BINARY, FK_NUMERIC, FK_UNKNOWN };

  Feature(std::string Name)
      : Kind(FeatureKind::FK_UNKNOWN), Name(std::move(Name)), Opt(false),
        Source(std::nullopt), Parent(nullptr) {}
  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] inline std::size_t getID() const {
    return std::hash<std::string>{}(getName().lower());
  }

  [[nodiscard]] FeatureKind getKind() const { return Kind; }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] bool isRoot() const { return Parent == nullptr; }

  [[nodiscard]] Feature *getParent() const { return Parent; }
  bool isParent(Feature *PosParent) const { return Parent == PosParent; }

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
  bool isImplied(Feature *PosImplication) const {
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

  [[nodiscard]] FeatureSourceRange *getFeatureSourceRange() {
    return Source.has_value() ? &Source.value() : nullptr;
  }
  void setFeatureSourceRange(FeatureSourceRange FeatureSR) {
    Source = std::move(FeatureSR);
  }

  //===--------------------------------------------------------------------===//
  // Utility

  [[nodiscard]] virtual std::string toString() const;

  LLVM_DUMP_METHOD
  void dump() const { llvm::outs() << toString() << '\n'; }

protected:
  Feature(FeatureKind Kind, string Name, bool Opt,
          std::optional<FeatureSourceRange> Source, Feature *Parent,
          FeatureSetType Children, FeatureSetType Excludes,
          FeatureSetType Implications, FeatureSetType Alternatives)
      : Kind(Kind), Name(std::move(Name)), Opt(Opt), Source(std::move(Source)),
        Parent(Parent), Children(std::move(Children)),
        Excludes(std::move(Excludes)), Implications(std::move(Implications)),
        Alternatives(std::move(Alternatives)) {}

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  void addChild(Feature *Feature) { Children.insert(Feature); }

  void setParent(Feature *Feature) { Parent = Feature; }

  void addExclude(Feature *F) { Excludes.insert(F); }

  void addAlternative(Feature *F) { Alternatives.insert(F); }

  void addImplication(Feature *F) { Implications.insert(F); }

  FeatureKind Kind;
  string Name;
  bool Opt;
  std::optional<FeatureSourceRange> Source;
  Feature *Parent;
  FeatureSetType Children;
  FeatureSetType Excludes;
  FeatureSetType Implications;
  FeatureSetType Alternatives;
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(string Name, bool Opt = false,
                std::optional<FeatureSourceRange> Loc = std::nullopt,
                Feature *Parent = nullptr, FeatureSetType Children = {},
                FeatureSetType Excludes = {}, FeatureSetType Implications = {},
                FeatureSetType Alternatives = {})
      : Feature(FeatureKind::FK_BINARY, std::move(Name), Opt, std::move(Loc),
                Parent, std::move(Children), std::move(Excludes),
                std::move(Implications), std::move(Alternatives)) {}

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

  NumericFeature(string Name, ValuesVariantType Values, bool Opt = false,
                 std::optional<FeatureSourceRange> Loc = std::nullopt,
                 Feature *Parent = nullptr, FeatureSetType Children = {},
                 FeatureSetType Excludes = {}, FeatureSetType Implications = {},
                 FeatureSetType Alternatives = {})
      : Feature(FeatureKind::FK_NUMERIC, std::move(Name), Opt, std::move(Loc),
                Parent, std::move(Children), std::move(Excludes),
                std::move(Implications), std::move(Alternatives)),
        Values(std::move(Values)) {}

  [[nodiscard]] ValuesVariantType getValues() const { return Values; }

  [[nodiscard]] string toString() const override;

  static bool classof(const Feature *F) {
    return F->getKind() == FeatureKind::FK_NUMERIC;
  }

private:
  ValuesVariantType Values;
};
} // namespace vara::feature

#endif // VARA_FEATURE_FEATURE_H
