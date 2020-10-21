#ifndef VARA_FEATURE_FEATURE_H
#define VARA_FEATURE_FEATURE_H

#include "vara/Feature/FeatureSourceRange.h"
#include "vara/Feature/Node.h"

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

/// \brief Base class for components of \a FeatureModel.
class Feature : public Node {
public:
  enum class FeatureKind { FK_BINARY, FK_NUMERIC, FK_UNKNOWN };

  Feature(std::string Name)
      : Node(NodeKind::NK_FEATURE), Kind(FeatureKind::FK_UNKNOWN),
        Name(std::move(Name)), Opt(false), Source(std::nullopt) {}
  Feature(const Feature &) = delete;
  Feature &operator=(const Feature &) = delete;
  virtual ~Feature() = default;

  [[nodiscard]] inline std::size_t hash() const {
    return std::hash<std::string>{}(getName().lower());
  }

  [[nodiscard]] FeatureKind getKind() const { return Kind; }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] bool isOptional() const { return Opt; }

  [[nodiscard]] Feature *getParentFeature() const {
    for (auto *P = this->getParent(); P; P = P->getParent()) {
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

  static bool classof(const Node *N) {
    return N->getNodeKind() == NodeKind::NK_FEATURE;
  }

protected:
  Feature(FeatureKind Kind, string Name, bool Opt,
          std::optional<FeatureSourceRange> Source, Node *Parent,
          NodeSetType Children)
      : Node(NodeKind::NK_FEATURE, Parent, std::move(Children)), Kind(Kind),
        Name(std::move(Name)), Opt(Opt), Source(std::move(Source)) {}

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  FeatureKind Kind;
  string Name;
  bool Opt;
  std::optional<FeatureSourceRange> Source;
};

/// Options without arguments.
class BinaryFeature : public Feature {

public:
  BinaryFeature(string Name, bool Opt = false,
                std::optional<FeatureSourceRange> Loc = std::nullopt,
                Feature *Parent = nullptr, const NodeSetType &Children = {})
      : Feature(FeatureKind::FK_BINARY, std::move(Name), Opt, std::move(Loc),
                Parent, Children) {}

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
                 Feature *Parent = nullptr, const NodeSetType &Children = {})
      : Feature(FeatureKind::FK_NUMERIC, std::move(Name), Opt, std::move(Loc),
                Parent, Children),
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
