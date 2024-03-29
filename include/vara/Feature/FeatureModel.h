#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/Error.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/Relationship.h"
#include "vara/Utils/Result.h"
#include "vara/Utils/UniqueIterator.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/GraphWriter.h"

#include <algorithm>
#include <numeric>
#include <queue>
#include <utility>

namespace vara::feature {

namespace detail {
class FeatureModelModification;
} // namespace detail

//===----------------------------------------------------------------------===//
//                               FeatureModel
//===----------------------------------------------------------------------===//

/// \brief Tree like representation of features and dependencies.
class FeatureModel {
  friend class FeatureModelBuilder;
  // Only Modifications are allowed to edit a FeatureModel after creation.
  friend class detail::FeatureModelModification;

public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<Feature>>;
  using RelationshipContainerTy = std::vector<std::unique_ptr<Relationship>>;

  FeatureModel(
      std::string Name = "FeatureModel",
      std::unique_ptr<RootFeature> Root = std::make_unique<RootFeature>("root"),
      fs::path Path = "", std::string Commit = "")
      : Name(std::move(Name)), Root(Root.get()), Path(std::move(Path)),
        Commit(std::move(Commit)) {
    addFeature(std::move(Root));
  }

  [[nodiscard]] unsigned int size() const { return Features.size(); }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] fs::path getPath() const { return Path; }

  [[nodiscard]] llvm::StringRef getCommit() const { return Commit; }

  //===--------------------------------------------------------------------===//
  // Features

  [[nodiscard]] RootFeature *getRoot() const { return Root; }

  [[nodiscard]] Feature *getFeature(llvm::StringRef F) const {
    auto SearchFeature = Features.find(F);
    if (SearchFeature != Features.end()) {
      return SearchFeature->getValue().get();
    }
    return nullptr;
  }

  //===--------------------------------------------------------------------===//
  // DFS feature iterator

  class DFSIterator {

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Feature *;
    using difference_type = ptrdiff_t;
    using pointer = Feature **;
    using reference = Feature *;

    using const_pointer = const Feature *const *;
    using const_reference = const Feature *;

    DFSIterator(Feature *F = nullptr) {
      if (F) {
        Frontier.push(F);
      }
    }
    DFSIterator(const DFSIterator &) = default;
    DFSIterator &operator=(const DFSIterator &) = delete;
    DFSIterator(DFSIterator &&) = default;
    DFSIterator &operator=(DFSIterator &&) = delete;
    ~DFSIterator() = default;

    reference operator*() {
      return Frontier.empty() ? nullptr : Frontier.top();
    }

    const_reference operator*() const {
      return Frontier.empty() ? nullptr : Frontier.top();
    }

    pointer operator->() {
      return Frontier.empty() ? nullptr : &Frontier.top();
    }

    const_pointer operator->() const {
      return Frontier.empty() ? nullptr : &Frontier.top();
    }

    DFSIterator operator++() {
      if (Frontier.empty()) {
        return *this;
      }
      auto *F = Frontier.top();
      Frontier.pop();
      if (F) {
        Visited.insert(F);
        llvm::SmallVector<Feature *, 3> Children;
        for (auto *C : F->getChildren<Feature>()) {
          Children.insert(std::upper_bound(Children.begin(), Children.end(), C,
                                           [](Feature *A, Feature *B) {
                                             return A->getName().lower() >
                                                    B->getName().lower();
                                             ;
                                           }),
                          C);
        }
        std::for_each(Children.begin(), Children.end(), [this](Feature *C) {
          if (Visited.find(C) == Visited.end()) {
            Frontier.push(C);
          }
        });
      }
      return *this;
    }

    DFSIterator operator++(int) {
      auto Iter(*this);
      ++*this;
      return Iter;
    }

    bool operator==(const DFSIterator &Other) const {
      if (Frontier.empty() || Other.Frontier.empty()) {
        return Frontier.empty() && Other.Frontier.empty();
      }
      return *this->Frontier.top() == *Other.Frontier.top();
    };

    bool operator!=(const DFSIterator &Other) const {
      return !(*this == Other);
    };

  private:
    std::stack<Feature *> Frontier;
    std::set<Feature *> Visited;
  };

  using const_ordered_feature_iterator = DFSIterator;

  [[nodiscard]] const_ordered_feature_iterator begin() const { return {Root}; }
  [[nodiscard]] const_ordered_feature_iterator end() const {
    return {Root ? Root->getParentFeature() : nullptr};
  }

  [[nodiscard]] llvm::iterator_range<const_ordered_feature_iterator>
  features() const {
    return llvm::make_range(begin(), end());
  }

  //===--------------------------------------------------------------------===//
  // Unordered feature iterator

  class FeatureMapIterator {

  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Feature *;
    using difference_type = ptrdiff_t;
    using pointer = Feature *;
    using reference = Feature *;

    using const_pointer = const Feature *;
    using const_reference = const Feature *;

    FeatureMapIterator(FeatureMapTy::const_iterator MapIter)
        : MapIter(MapIter) {}
    FeatureMapIterator(const FeatureMapIterator &) = default;
    FeatureMapIterator &operator=(const FeatureMapIterator &) = delete;
    FeatureMapIterator(FeatureMapIterator &&) = default;
    FeatureMapIterator &operator=(FeatureMapIterator &&) = delete;
    ~FeatureMapIterator() = default;

    reference operator*() { return MapIter->getValue().get(); }
    const_reference operator*() const { return MapIter->getValue().get(); }

    pointer operator->() { return **this; }
    const_pointer operator->() const { return **this; }

    FeatureMapIterator operator++() {
      ++MapIter;
      return *this;
    }

    FeatureMapIterator operator++(int) {
      auto Iter(*this);
      ++*this;
      return Iter;
    }

    bool operator==(const FeatureMapIterator &Other) const {
      return MapIter == Other.MapIter;
    };

    bool operator!=(const FeatureMapIterator &Other) const {
      return !(*this == Other);
    };

  private:
    FeatureMapTy::const_iterator MapIter;
  };

  using const_unordered_feature_iterator = FeatureMapIterator;

  [[nodiscard]] llvm::iterator_range<const_unordered_feature_iterator>
  unorderedFeatures() const {
    return llvm::make_range(FeatureMapIterator(Features.begin()),
                            FeatureMapIterator(Features.end()));
  }

  //===--------------------------------------------------------------------===//
  // Constraints

private:
  /// \brief Base for different constraint kinds (boolean, non-boolean, etc.).
  class FeatureModelConstraint {
  public:
    virtual ~FeatureModelConstraint() = default;

    [[nodiscard]] Constraint *constraint() { return C.get(); }

    [[nodiscard]] Constraint *operator*() { return C.get(); }

    [[nodiscard]] virtual std::string toString() const { return C->toString(); }

    [[nodiscard]] virtual std::string toHTML() const { return C->toHTML(); }

  protected:
    FeatureModelConstraint(std::unique_ptr<Constraint> C) : C(std::move(C)) {}

  private:
    std::unique_ptr<Constraint> C;
  };

public:
  class BooleanConstraint : public FeatureModelConstraint {
  public:
    BooleanConstraint(std::unique_ptr<Constraint> C)
        : FeatureModelConstraint(std::move(C)) {}
  };

  using BooleanConstraintContainerTy =
      std::vector<std::unique_ptr<BooleanConstraint>>;
  using const_boolean_constraint_iterator =
      UniqueIterator<const BooleanConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<const_boolean_constraint_iterator>
  booleanConstraints() const {
    return llvm::make_range(
        const_boolean_constraint_iterator(BooleanConstraints.begin()),
        const_boolean_constraint_iterator(BooleanConstraints.end()));
  }

  class NonBooleanConstraint : public FeatureModelConstraint {
  public:
    NonBooleanConstraint(std::unique_ptr<Constraint> C)
        : FeatureModelConstraint(std::move(C)) {}
  };

  using NonBooleanConstraintContainerTy =
      std::vector<std::unique_ptr<NonBooleanConstraint>>;
  using const_non_boolean_constraint_iterator =
      UniqueIterator<const NonBooleanConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<const_non_boolean_constraint_iterator>
  nonBooleanConstraints() const {
    return llvm::make_range(
        const_non_boolean_constraint_iterator(NonBooleanConstraints.begin()),
        const_non_boolean_constraint_iterator(NonBooleanConstraints.end()));
  }

  class MixedConstraint : public FeatureModelConstraint {
  public:
    /// This enum indicates whether the constraint should be evaluated when
    /// `all` boolean features are present or `none`.
    enum class Req { ALL, NONE };

    /// This enum indicates whether the whole constraint should be `negated`
    /// or not.
    enum class ExprKind { POS, NEG };

    MixedConstraint(std::unique_ptr<Constraint> C, Req R, ExprKind E)
        : FeatureModelConstraint(std::move(C)), R(R), E(E) {}

    [[nodiscard]] Req req() { return R; }

    [[nodiscard]] ExprKind exprKind() { return E; }

  private:
    Req R;
    ExprKind E;
  };

  using MixedConstraintContainerTy =
      std::vector<std::unique_ptr<MixedConstraint>>;
  using const_mixed_constraint_iterator =
      UniqueIterator<const MixedConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<const_mixed_constraint_iterator>
  mixedConstraints() const {
    return llvm::make_range(
        const_mixed_constraint_iterator(MixedConstraints.begin()),
        const_mixed_constraint_iterator(MixedConstraints.end()));
  }

  //===--------------------------------------------------------------------===//
  // Relationships

  using const_relationship_iterator =
      typename RelationshipContainerTy::const_iterator;

  [[nodiscard]] llvm::iterator_range<const_relationship_iterator>
  relationships() const {
    return llvm::make_range(Relationships.begin(), Relationships.end());
  }

  //===--------------------------------------------------------------------===//
  // Utility

  void view() { ViewGraph(this, "FeatureModel-" + this->getName()); }

  /// Create deep clone of whole data structure.
  ///
  /// \return new \a FeatureModel
  [[nodiscard]] std::unique_ptr<FeatureModel> clone() const;

  LLVM_DUMP_METHOD
  void dump() const;

private:
  void setName(std::string NewName) { Name = std::move(NewName); }

  void setPath(fs::path NewPath) { Path = std::move(NewPath); }

  void setCommit(std::string NewCommit) { Commit = std::move(NewCommit); }

  //===--------------------------------------------------------------------===//
  // Features

  /// Insert a \a Feature into existing model.
  ///
  /// \param[in] Feature feature to be inserted
  ///
  /// \returns ptr to inserted \a Feature
  Feature *addFeature(std::unique_ptr<Feature> Feature);

  /// Delete a \a Feature.
  void removeFeature(Feature &Feature);

  RootFeature *setRoot(RootFeature &NewRoot);

  using ordered_feature_iterator = DFSIterator;

  [[nodiscard]] ordered_feature_iterator begin() { return {Root}; }
  [[nodiscard]] ordered_feature_iterator end() {
    return {Root ? Root->getParentFeature() : nullptr};
  }

  [[nodiscard]] llvm::iterator_range<ordered_feature_iterator> features() {
    return llvm::make_range(begin(), end());
  }

  using unordered_feature_iterator = FeatureMapIterator;

  [[nodiscard]] llvm::iterator_range<unordered_feature_iterator>
  unorderedFeatures() {
    return llvm::make_range(FeatureMapIterator(Features.begin()),
                            FeatureMapIterator(Features.end()));
  }

  //===--------------------------------------------------------------------===//
  // Constraints

  vara::feature::Constraint *
  addConstraint(std::unique_ptr<BooleanConstraint> C) {
    BooleanConstraints.push_back(std::move(C));
    return **BooleanConstraints.back();
  }

  using boolean_constraint_iterator =
      UniqueIterator<BooleanConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<boolean_constraint_iterator>
  booleanConstraints() {
    return llvm::make_range(
        boolean_constraint_iterator(BooleanConstraints.begin()),
        boolean_constraint_iterator(BooleanConstraints.end()));
  }

  vara::feature::Constraint *
  addConstraint(std::unique_ptr<NonBooleanConstraint> C) {
    NonBooleanConstraints.push_back(std::move(C));
    return **NonBooleanConstraints.back();
  }

  using non_boolean_constraint_iterator =
      UniqueIterator<NonBooleanConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<non_boolean_constraint_iterator>
  nonBooleanConstraints() {
    return llvm::make_range(
        non_boolean_constraint_iterator(NonBooleanConstraints.begin()),
        non_boolean_constraint_iterator(NonBooleanConstraints.end()));
  }

  vara::feature::Constraint *addConstraint(std::unique_ptr<MixedConstraint> C) {
    MixedConstraints.push_back(std::move(C));
    return **MixedConstraints.back();
  }

  using mixed_constraint_iterator = UniqueIterator<MixedConstraintContainerTy>;

  [[nodiscard]] llvm::iterator_range<mixed_constraint_iterator>
  mixedConstraints() {
    return llvm::make_range(mixed_constraint_iterator(MixedConstraints.begin()),
                            mixed_constraint_iterator(MixedConstraints.end()));
  }

  //===--------------------------------------------------------------------===//
  // Relationships

  Relationship *addRelationship(std::unique_ptr<Relationship> Relationship) {
    Relationships.push_back(std::move(Relationship));
    return Relationships.back().get();
  }

  void removeRelationship(Relationship *R) {
    Relationships.erase(
        std::find_if(Relationships.begin(), Relationships.end(),
                     [R](const std::unique_ptr<Relationship> &UniR) {
                       return UniR.get() == R;
                     }));
  }

  using relationship_iterator = typename RelationshipContainerTy::iterator;

  [[nodiscard]] llvm::iterator_range<relationship_iterator> relationships() {
    return llvm::make_range(Relationships.begin(), Relationships.end());
  }

  std::string Name;
  RootFeature *Root;
  fs::path Path;
  std::string Commit;
  FeatureMapTy Features;
  BooleanConstraintContainerTy BooleanConstraints;
  NonBooleanConstraintContainerTy NonBooleanConstraints;
  MixedConstraintContainerTy MixedConstraints;
  RelationshipContainerTy Relationships;
};

//===----------------------------------------------------------------------===//
//                           FeatureModel Helpers
//===----------------------------------------------------------------------===//

std::unique_ptr<FeatureModel> loadFeatureModel(llvm::StringRef Path);

bool verifyFeatureModel(llvm::StringRef Path);

} // namespace vara::feature

namespace llvm {

//===----------------------------------------------------------------------===//
//                     GraphWriter for FeatureModel
//===----------------------------------------------------------------------===//

template <>
struct GraphWriter<vara::feature::FeatureModel *> {
  using GraphType = typename vara::feature::FeatureModel *;

  raw_ostream &O;
  const GraphType &G;

  using NodeRef = typename vara::feature::FeatureTreeNode *;

  GraphWriter(raw_ostream &O, const GraphType &G, bool SN) : O(O), G(G) {}

  void writeGraph(const std::string &Title = "") const {
    // Output the header for the graph
    writeHeader(Title);

    // Emit all of the nodes in the graph
    writeNodes();

    // Output the end of the graph
    writeFooter();
  }

  void writeHeader(const std::string &Title) const {
    if (!Title.empty()) {
      O << "digraph \"" << DOT::EscapeString(Title) << "\" {\n";
    } else {
      O << "digraph graph_" << static_cast<void *>(G) << " {\n";
    }
    std::string GraphName =
        llvm::formatv("Feature model for {0}", G->getName().str());

    O.indent(2) << "graph [pad=.5 nodesep=2 ranksep=2 splines=true "
                   "newrank=true bgcolor=white rankdir=tb overlap=false "
                   "fontname=\"CMU Typewriter\" label=\""
                << DOT::EscapeString(GraphName) << "\"];\n\n";
  }

  /// Output tree structure of feature model and additional edges.
  void writeNodes() const { emitCluster(G->getRoot()); }

  void writeFooter() const { O << "}\n"; }

  using FeatureEdgeSetTy = llvm::SmallSet<
      std::pair<vara::feature::Feature *, vara::feature::Feature *>, 10>;

  /// Checks whether an edge would be a duplicate.
  ///
  /// \param[in] Edge may be already visited.
  /// \param[in] Skip contains existing edges.
  static bool
  visited(std::pair<vara::feature::Feature *, vara::feature::Feature *> Edge,
          const FeatureEdgeSetTy &Skip) {
    return std::any_of(Skip.begin(), Skip.end(), [Edge](const auto &P) {
      return P.first == Edge.first && P.second == Edge.second;
    });
  }

  /// Output feature model (tree) recursively.
  ///
  /// \param[in] Node Root of subtree.
  /// \param[in] Indent Value to indent statements in dot file.
  void emitCluster(NodeRef Node, const int Indent = 0) const {
    O.indent(Indent);
    emitNode(Node);
    if (Node->begin() != Node->end()) {
      O.indent(Indent + 2) << "subgraph cluster_" << static_cast<void *>(Node)
                           << " {\n";
      O.indent(Indent + 4) << "label=\"\";\n";
      O.indent(Indent + 4) << "margin=0;\n";
      O.indent(Indent + 4) << "style=invis;\n";
      for (auto *Child : *Node) {
        emitCluster(Child, Indent + 2);
        O.indent(Indent + 2);
        if (auto *F = llvm::dyn_cast<vara::feature::Feature>(Child); F) {
          emitEdge(
              Node, F,
              llvm::formatv("arrowhead={0}", F->isOptional() ? "odot" : "dot"));
        } else {
          emitEdge(Node, Child, "arrowhead=none");
        }
      }
      O.indent(Indent + 4) << "{\n";
      O.indent(Indent + 6) << "rank=same;\n";
      for (auto *Child : *Node) {
        O.indent(Indent + 6) << "node_" << static_cast<void *>(Child) << ";\n";
      }
      O.indent(Indent + 4) << "}\n";
      O.indent(Indent + 2) << "}\n";
    }
  }

  static std::string buildFeatureLabel(vara::feature::Feature *F) {
    std::stringstream CS;
    for (const auto &C : F->constraints()) {
      CS << "<tr><td>" << DOT::EscapeString(C->getRoot()->toHTML())
         << "</td></tr>";
    }

    std::stringstream LS;
    if (F->hasLocations()) {
      LS << "<hr/>";
      for (const auto &Location : F->getLocations()) {
        LS << llvm::formatv("<tr><td><b>{0}</b></td></tr>",
                            DOT::EscapeString(Location.toString()))
                  .str();
      }
    }

    return llvm::formatv(
        "<<table align=\"center\" valign=\"middle\" border=\"0\" "
        "cellborder=\"0\" "
        "cellpadding=\"5\">{0}{1}{2}</table>>",
        llvm::formatv("<tr><td><b>{0} (Flag: {1})</b></td></tr>",
                      DOT::EscapeString(F->getName().str()),
                      F->getOutputString().trim().str()),
        CS.str(), LS.str());
  }

  /// Output \a Feature node with custom attributes.
  void emitNode(NodeRef Node) const {
    std::string Label;
    std::string Shape;
    if (auto *F = llvm::dyn_cast<vara::feature::Feature>(Node); F) {
      Shape = "box";
      Label = buildFeatureLabel(F);
    } else {
      Shape = "ellipse";
      if (auto *R = llvm::dyn_cast<vara::feature::Relationship>(Node); R) {
        switch (R->getKind()) {
        case vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE:
          Label = "ALTERNATIVE";
          break;
        case vara::feature::Relationship::RelationshipKind::RK_OR:
          Label = "OR";
          break;
        }
      } else {
        Label = "error";
      }
    }
    O.indent(2) << "node_" << static_cast<void *>(Node) << " ["
                << "shape=" << Shape
                << " margin=.1 fontsize=12 fontname=\"CMU "
                   "Typewriter\" label="
                << Label << "];\n";
  }

  void emitEdge(NodeRef SrcNode, NodeRef DestNode,
                const std::string &Attrs = "") const {
    O.indent(2) << "node_" << static_cast<void *>(SrcNode) << " -> node_"
                << static_cast<void *>(DestNode);
    if (!Attrs.empty()) {
      O << " [" << Attrs << "]";
    }
    O << ";\n";
  }
};

} // namespace llvm

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelConsistencyRules
//===----------------------------------------------------------------------===//

template <typename... Rules>
class FeatureModelConsistencyChecker {
public:
  static Result<FTErrorCode> isFeatureModelValid(const FeatureModel &FM) {
    if (auto E = (Rules::check(FM) && ... && true); !E) {
      return Error(INCONSISTENT);
    }
    return Ok();
  }
};

struct EveryFeatureRequiresParent {
  static bool check(const FeatureModel &FM) {
    if (std::all_of(FM.unorderedFeatures().begin(),
                    FM.unorderedFeatures().end(), [](Feature *F) {
                      return llvm::isa<RootFeature>(F) || F->getParentFeature();
                    })) {
      return true;
    }
    llvm::errs() << "Failed to validate 'EveryFeatureRequiresParent'." << '\n';
    return false;
  }
};

struct CheckFeatureParentChildRelationShip {
  static bool check(const FeatureModel &FM) {
    if (std::all_of(
            FM.unorderedFeatures().begin(), FM.unorderedFeatures().end(),
            [](Feature *F) {
              return llvm::isa<RootFeature>(F) ||
                     // Every parent of a Feature needs to have it as a child.
                     std::any_of(
                         F->getParent()->begin(), F->getParent()->end(),
                         [F](FeatureTreeNode *Child) { return F == Child; });
            })) {
      return true;
    }
    llvm::errs() << "Failed to validate 'CheckFeatureParentChildRelationShip'."
                 << '\n';
    return false;
  }
};

struct ExactlyOneRootNode {
  static bool check(const FeatureModel &FM) {
    if (llvm::isa_and_nonnull<RootFeature>(FM.getRoot()) &&
        1 == std::accumulate(FM.unorderedFeatures().begin(),
                             FM.unorderedFeatures().end(), 0,
                             [](int Sum, Feature *F) {
                               return Sum + llvm::isa<RootFeature>(F);
                             })) {
      return true;
    }
    llvm::errs() << "Failed to validate 'ExactlyOneRootNode'." << '\n';
    return false;
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODEL_H
