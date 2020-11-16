#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/OrderedFeatureVector.h"
#include "vara/Feature/Relationship.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/GraphWriter.h"

#include <algorithm>
#include <queue>
#include <utility>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureModel
//===----------------------------------------------------------------------===//

/// \brief Tree like representation of features and dependencies.
class FeatureModel {
public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<Feature>>;
  using OrderedFeatureTy = OrderedFeatureVector;
  using ConstraintTy = Constraint;
  using ConstraintContainerTy = std::vector<std::unique_ptr<ConstraintTy>>;
  using RelationshipTy = Relationship;
  using RelationshipContainerTy = std::vector<std::unique_ptr<RelationshipTy>>;

  FeatureModel(string Name, fs::path RootPath, FeatureMapTy Features,
               ConstraintContainerTy Constraints,
               RelationshipContainerTy Relationships, Feature *Root)
      : Name(std::move(Name)), Path(std::move(RootPath)),
        Features(std::move(Features)), Constraints(std::move(Constraints)),
        Relationships(std::move(Relationships)), Root(Root) {
    // Insert all values into ordered data structure.
    for (const auto &KV : this->Features) {
      OrderedFeatures.insert(KV.getValue().get());
    }
  }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] fs::path getPath() const { return Path; }

  [[nodiscard]] Feature *getRoot() const {
    assert(Root);
    return Root;
  }

  /// Insert a \a Feature into existing model while keeping consistency and
  /// ordering.
  ///
  /// \param[in] Feature feature to be inserted
  /// \return if feature was inserted successfully
  bool addFeature(std::unique_ptr<Feature> Feature);

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

  Feature *getFeature(llvm::StringRef F) { return Features[F].get(); }

  LLVM_DUMP_METHOD
  void dump() const;

protected:
  string Name;
  fs::path Path;
  FeatureMapTy Features;
  ConstraintContainerTy Constraints;
  RelationshipContainerTy Relationships;
  Feature *Root{nullptr};

  FeatureModel() = default;

private:
  OrderedFeatureTy OrderedFeatures;
};

//===----------------------------------------------------------------------===//
//                     Builder for FeatureModel
//===----------------------------------------------------------------------===//

/// \brief Builder for \a FeatureModel which can be used while parsing.
class FeatureModelBuilder : private FeatureModel {
public:
  void init() {
    Name = "";
    Path = "";
    Root = nullptr;
    Features.clear();
    Constraints.clear();
    Parents.clear();
    Children.clear();
  }

  /// Try to create and add a new \a Feature to the \a FeatureModel.
  ///
  /// \param[in] FeatureName name of the \a Feature
  /// \param[in] FurtherArgs further arguments that should be passed to the
  ///                        \a Feature constructor
  ///
  /// \returns true, if the feature could be inserted into the \a FeatureModel
  template <typename FeatureTy, typename... Args,
            typename = typename std::enable_if_t<
                std::is_base_of_v<Feature, FeatureTy>, int>>
  bool makeFeature(const std::string &FeatureName, Args... FurtherArgs) {
    return Features
        .try_emplace(FeatureName, std::make_unique<FeatureTy>(
                                      FeatureName, std::move(FurtherArgs)...))
        .second;
  }

  bool addFeature(Feature &F);

  FeatureModelBuilder *addParent(const std::string &FeatureName,
                                 const std::string &ParentName) {
    Children[ParentName].insert(FeatureName);
    Parents[FeatureName] = ParentName;
    return this;
  }

  FeatureModelBuilder *
  addRelationship(Relationship::RelationshipKind RK,
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

  FeatureModelBuilder *setRoot(const std::string &RootName = "root");

  /// Build \a FeatureModel.
  ///
  /// @return instance of \a FeatureModel

  std::unique_ptr<FeatureModel> buildFeatureModel();

  /// Build simple \a FeatureModel from given edges.
  ///
  /// \param[in] B edges with \a BinaryFeature
  /// \param[in] N edges with \a NumericFeature
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildSimpleFeatureModel(
      const std::initializer_list<std::pair<std::string, std::string>> &B,
      const std::initializer_list<std::pair<
          std::string,
          std::pair<std::string, NumericFeature::ValuesVariantType>>> &N = {});

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
    for (const auto &P : Skip) {
      if (P.first == Edge.first && P.second == Edge.second) {
        return true;
      }
    }
    return false;
  }

  /// Output feature model (tree) recursively.
  ///
  /// \param[in] Node Root of subtree.
  /// \param[in] Indent Value to indent statements in dot file.
  void emitCluster(const NodeRef Node, const int Indent = 0) {
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
        auto *F = llvm::dyn_cast<vara::feature::Feature>(Child);
        if (F) {
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
  void emitNode(const NodeRef Node) {
    std::string Label;
    auto *F = llvm::dyn_cast<vara::feature::Feature>(Node);
    if (F) {
      std::stringstream CS;
      for (const auto &C : F->constraints()) {
        CS << "<tr><td>" << DOT::EscapeString(C->getRoot()->toHTML())
           << "</td></tr>";
      }
      Label =
          llvm::formatv(
              "<<table align=\"center\" valign=\"middle\" border=\"0\" "
              "cellborder=\"0\" "
              "cellpadding=\"5\">{0}{1}{2}</table>>",
              llvm::formatv("<tr><td><b>{0}</b></td></tr>",
                            DOT::EscapeString(F->getName().str()))
                  .str(),
              CS.str(),
              (F->getFeatureSourceRange()
                   ? llvm::formatv("<hr/><tr><td>{0}</td></tr>",
                                   DOT::EscapeString(
                                       F->getFeatureSourceRange()->toString()))
                         .str()
                   : ""))
              .str();
    } else {
      auto *R = llvm::dyn_cast<vara::feature::Relationship>(Node);
      if (R) {
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
                << "shape=box margin=.1 fontsize=12 fontname=\"CMU "
                   "Typewriter\" label="
                << Label << "];\n";
  }

  void emitEdge(const NodeRef SrcNode, const NodeRef DestNode,
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

#endif // VARA_FEATURE_FEATUREMODEL_H
