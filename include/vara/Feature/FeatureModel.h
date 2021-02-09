#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/FeatureTreeNode.h"
#include "vara/Feature/OrderedFeatureVector.h"
#include "vara/Feature/Relationship.h"

#include "llvm/ADT/SmallSet.h"
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
  // Only Modifications are allowed to edit a FeatureModel after creation.
  friend class detail::FeatureModelModification;

public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<Feature>>;
  using OrderedFeatureTy = OrderedFeatureVector;
  using ConstraintTy = Constraint;
  using ConstraintContainerTy = std::vector<std::unique_ptr<ConstraintTy>>;
  using RelationshipTy = Relationship;
  using RelationshipContainerTy = std::vector<std::unique_ptr<RelationshipTy>>;

  FeatureModel(std::string Name, fs::path RootPath, std::string Commit,
               FeatureMapTy Features, ConstraintContainerTy Constraints,
               RelationshipContainerTy Relationships, Feature *Root)
      : Name(std::move(Name)), Path(std::move(RootPath)),
        Commit(std::move(Commit)), Features(std::move(Features)),
        Constraints(std::move(Constraints)),
        Relationships(std::move(Relationships)), Root(Root) {
    // Insert all values into ordered data structure.
    for (const auto &KV : this->Features) {
      OrderedFeatures.insert(KV.getValue().get());
    }
  }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] fs::path getPath() const { return Path; }

  [[nodiscard]] llvm::StringRef getCommit() const { return Commit; }

  [[nodiscard]] Feature *getRoot() const {
    assert(Root);
    return Root;
  }

  //===--------------------------------------------------------------------===//
  // Ordered feature iterator
  OrderedFeatureVector::ordered_feature_iterator begin() {
    return OrderedFeatures.begin();
  }
  [[nodiscard]] OrderedFeatureVector::const_ordered_feature_iterator
  begin() const {
    return OrderedFeatures.begin();
  }

  OrderedFeatureVector::ordered_feature_iterator end() {
    return OrderedFeatures.end();
  }
  [[nodiscard]] OrderedFeatureVector::const_ordered_feature_iterator
  end() const {
    return OrderedFeatures.end();
  }

  llvm::iterator_range<OrderedFeatureVector::ordered_feature_iterator>
  features() {
    return llvm::make_range(begin(), end());
  }
  [[nodiscard]] llvm::iterator_range<
      OrderedFeatureVector::const_ordered_feature_iterator>
  features() const {
    return llvm::make_range(begin(), end());
  }

  //===--------------------------------------------------------------------===//
  // Utility

  void view() { ViewGraph(this, "FeatureModel-" + this->getName()); }

  [[nodiscard]] Feature *getFeature(llvm::StringRef F) const {
    auto SearchFeature = Features.find(F);
    if (SearchFeature != Features.end()) {
      return SearchFeature->getValue().get();
    }
    return nullptr;
  }

  /// Create deep clone of whole data structure.
  ///
  /// \return new \a FeatureModel
  std::unique_ptr<FeatureModel> clone();

  LLVM_DUMP_METHOD
  void dump() const;

protected:
  std::string Name;
  fs::path Path;
  std::string Commit;
  FeatureMapTy Features;
  ConstraintContainerTy Constraints;
  RelationshipContainerTy Relationships;
  Feature *Root{nullptr};

  FeatureModel() = default;

private:
  /// Insert a \a Feature into existing model.
  ///
  /// \param[in] Feature feature to be inserted
  ///
  /// \returns ptr to inserted \a Feature
  Feature *addFeature(std::unique_ptr<Feature> Feature);

  /// Delete a \a Feature.
  void removeFeature(Feature &Feature);

  OrderedFeatureTy OrderedFeatures;
};

//===----------------------------------------------------------------------===//
//                     Builder for FeatureModel
//===----------------------------------------------------------------------===//

/// \brief Builder for \a FeatureModel which can be used while parsing.
class FeatureModelBuilder : private FeatureModel {
public:
  FeatureModelBuilder() = default;
  void init() {
    Name = "";
    Path = "";
    Commit = "";
    Root = nullptr;
    Features.clear();
    Constraints.clear();
    Parents.clear();
    Children.clear();
  }

  /// Try to create a new \a Feature.
  ///
  /// \param[in] FeatureName name of the \a Feature
  /// \param[in] FurtherArgs further arguments that should be passed to the
  ///                        \a Feature constructor
  ///
  /// \returns ptr to inserted \a Feature
  template <typename FeatureTy, typename... Args,
            typename = typename std::enable_if_t<
                std::is_base_of_v<Feature, FeatureTy>, int>>
  Feature *makeFeature(const std::string &FeatureName, Args... FurtherArgs) {
    if (!Features
             .try_emplace(FeatureName,
                          std::make_unique<FeatureTy>(
                              FeatureName, std::move(FurtherArgs)...))
             .second) {
      return nullptr;
    }
    return Features[FeatureName].get();
  }

  FeatureModelBuilder *addEdge(const std::string &ParentName,
                               const std::string &FeatureName) {
    Children[ParentName].insert(FeatureName);
    Parents[FeatureName] = ParentName;
    return this;
  }

  FeatureModelBuilder *
  emplaceRelationship(Relationship::RelationshipKind RK,
                      const std::vector<std::string> &FeatureNames,
                      const std::string &ParentName) {
    RelationshipEdges[ParentName].emplace_back(RK, FeatureNames);
    return this;
  }

  FeatureModelBuilder *
  addConstraint(std::unique_ptr<FeatureModel::ConstraintTy> C) {
    Constraints.push_back(std::move(C));
    return this;
  }

  FeatureModelBuilder *setVmName(std::string Name) {
    this->Name = std::move(Name);
    return this;
  }

  FeatureModelBuilder *setPath(fs::path Path) {
    this->Path = std::move(Path);
    return this;
  }

  FeatureModelBuilder *setCommit(std::string Commit) {
    this->Commit = std::move(Commit);
    return this;
  }

  FeatureModelBuilder *setRoot(const std::string &RootName = "root");

  /// Build \a FeatureModel.
  ///
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();

private:
  class BuilderVisitor : public ConstraintVisitor {

  public:
    BuilderVisitor(FeatureModelBuilder *Builder) : Builder(Builder) {}

    void visit(PrimaryFeatureConstraint *C) override {
      auto *F = Builder->getFeature(C->getFeature()->getName());
      C->setFeature(F);
      F->addConstraint(C);
    };

  private:
    FeatureModelBuilder *Builder;
  };

  using EdgeMapType = typename llvm::StringMap<llvm::SmallSet<std::string, 3>>;
  using RelationshipEdgeType = typename llvm::StringMap<std::vector<
      std::pair<Relationship::RelationshipKind, std::vector<std::string>>>>;

  FeatureModel::ConstraintContainerTy Constraints;
  FeatureModel::RelationshipContainerTy Relationships;
  llvm::StringMap<std::string> Parents;
  EdgeMapType Children;
  RelationshipEdgeType RelationshipEdges;

  bool buildConstraints();

  /// This method is solely relevant for parsing XML, as alternatives are
  /// represented als mutual excluded but non-optional features (which requires
  /// additional processing).
  void detectXMLAlternatives();

  bool buildTree(const std::string &FeatureName,
                 std::set<std::string> &Visited);
};
} // namespace vara::feature

inline std::ostream &operator<<(std::ostream &Out,
                                const vara::feature::Feature *Feature) {
  Out << Feature->toString();
  return Out;
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Out,
                                     const vara::feature::Feature *Feature) {
  Out << Feature->toString();
  return Out;
}

namespace llvm {

//===----------------------------------------------------------------------===//
//                     GraphWriter for FeatureModel
//===----------------------------------------------------------------------===//

template <> struct GraphWriter<vara::feature::FeatureModel *> {
  using GraphType = typename vara::feature::FeatureModel *;

  raw_ostream &O;
  const GraphType &G;

  using NodeRef = typename vara::feature::FeatureTreeNode *;

  GraphWriter(raw_ostream &O, const GraphType &G, bool SN) : O(O), G(G) {}

  void writeGraph(const std::string &Title = "") {
    // Output the header for the graph
    writeHeader(Title);

    // Emit all of the nodes in the graph
    writeNodes();

    // Output the end of the graph
    writeFooter();
  }

  void writeHeader(const std::string &Title) {
    if (!Title.empty()) {
      O << "digraph \"" << DOT::EscapeString(Title) << "\" {\n";
    } else {
      O << "digraph graph_" << static_cast<void *>(G) << " {\n";
    }
    std::string GraphName =
        llvm::formatv("Feature model for {0}\n{1}", G->getName().str(),
                      G->getPath().string());

    O.indent(2) << "graph [pad=.5 nodesep=2 ranksep=2 splines=true "
                   "newrank=true bgcolor=white rankdir=tb overlap=false "
                   "fontname=\"CMU Typewriter\" label=\""
                << DOT::EscapeString(GraphName) << "\"];\n\n";
  }

  /// Output tree structure of feature model and additional edges.
  void writeNodes() { emitCluster(G->getRoot()); }

  void writeFooter() { O << "}\n"; }

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
  void emitCluster(NodeRef Node, const int Indent = 0) {
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

  /// Output \a Feature node with custom attributes.
  void emitNode(NodeRef Node) {
    std::string Label;
    std::string Shape;
    if (auto *F = llvm::dyn_cast<vara::feature::Feature>(Node); F) {
      Shape = "box";
      std::stringstream CS;
      for (const auto &C : F->constraints()) {
        CS << "<tr><td>" << DOT::EscapeString(C->getRoot()->toHTML())
           << "</td></tr>";
      }
      Label = llvm::formatv(
          "<<table align=\"center\" valign=\"middle\" border=\"0\" "
          "cellborder=\"0\" "
          "cellpadding=\"5\">{0}{1}{2}</table>>",
          llvm::formatv("<tr><td><b>{0}</b></td></tr>",
                        DOT::EscapeString(F->getName().str())),
          CS.str(),
          (F->hasLocations() ? llvm::formatv("<hr/><tr><td>{0}</td></tr>",
                                             DOT::EscapeString(""))
                                   .str()
                             : ""));
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
                const std::string &Attrs = "") {
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

template <typename... Rules> class FeatureModelConsistencyChecker {
public:
  static bool isFeatureModelValid(FeatureModel &FM) {
    return (Rules::check(FM) && ... && true);
  }
};

struct EveryFeatureRequiresParent {
  static bool check(FeatureModel &FM) {
    auto *Root = FM.getRoot();
    return Root && std::all_of(FM.begin(), FM.end(), [Root](Feature *F) {
             return (*Root == *F) || F->getParent();
           });
  }
};

struct CheckFeatureParentChildRelationShip {
  static bool check(FeatureModel &FM) {
    auto *Root = FM.getRoot();
    return Root && std::all_of(FM.begin(), FM.end(), [Root](Feature *F) {
             return (*Root == *F) ||
                    // Every parent of a Feature needs to have that as a child.
                    std::any_of(
                        F->getParent()->begin(), F->getParent()->end(),
                        [F](FeatureTreeNode *Child) { return F == Child; });
           });
  }
};

struct ExactlyOneRootNode {
  static bool check(FeatureModel &FM) {
    return FM.getRoot() && 1 == std::accumulate(FM.begin(), FM.end(), 0,
                                                [](int Sum, Feature *F) {
                                                  return Sum + !F->getParent();
                                                });
  }
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODEL_H
