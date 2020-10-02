#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/OrderedFeatureVector.h"

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
  using ConstraintTy = llvm::SmallVector<std::pair<std::string, bool>, 3>;
  using ConstraintsTy = std::vector<ConstraintTy>;

  FeatureModel(string Name, fs::path RootPath, FeatureMapTy Features,
               Feature *Root)
      : Name(std::move(Name)), Path(std::move(RootPath)), Root(Root),
        Features(std::move(Features)) {
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
  Feature *Root;

private:
  OrderedFeatureTy OrderedFeatures;
};

//===----------------------------------------------------------------------===//
//                     Builder for FeatureModel
//===----------------------------------------------------------------------===//

/// \brief Builder for \a FeatureModel which can be used while parsing.
class FeatureModelBuilder : private FeatureModel {
public:
  FeatureModelBuilder() : FeatureModel("", "", {}, nullptr){};

  void init() {
    Name = "";
    Path = "";
    Root = nullptr;
    Features.clear();
    Constraints.clear();
    Parents.clear();
    Children.clear();
    Excludes.clear();
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

  FeatureModelBuilder *addExclude(const std::string &FeatureName,
                                  const std::string &ExcludeName) {
    Excludes[FeatureName].insert(ExcludeName);
    return this;
  }

  FeatureModelBuilder *
  addConstraint(const FeatureModel::ConstraintTy &Constraint) {
    Constraints.push_back(Constraint);
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
      const std::vector<std::pair<std::string, std::string>> &B,
      const std::vector<std::pair<
          std::string,
          std::pair<std::string, NumericFeature::ValuesVariantType>>> &N = {});

private:
  using EdgeMapType = typename llvm::StringMap<llvm::SmallSet<std::string, 3>>;

  FeatureModel::ConstraintsTy Constraints;
  llvm::StringMap<std::string> Parents;
  EdgeMapType Children;
  EdgeMapType Excludes;

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

  using NodeRef = typename vara::feature::Feature *;

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
  void writeNodes() {
    emitCluster(G->getRoot());
    (O << '\n').indent(2) << "// Excludes\n";
    emitExcludeEdges();
    (O << '\n').indent(2) << "// Implications\n";
    emitImplicationEdges();
    (O << '\n').indent(2) << "// Alternatives\n";
    emitAlternativeEdges();
  }

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

  void emitExcludeEdges() {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *G) {
      for (const auto &Exclude : Node->excludes()) {
        if (visited(std::make_pair(Node, Exclude), Skip)) {
          continue;
        }
        if (std::find(Exclude->excludes_begin(), Exclude->excludes_end(),
                      Node) != Exclude->excludes_end()) {
          emitEdge(Node, Exclude, "color=red dir=both constraint=false");
          Skip.insert(std::make_pair<>(Exclude, Node));
        } else {
          emitEdge(Node, Exclude, "color=red");
        }
        Skip.insert(std::make_pair<>(Node, Exclude));
      }
    }
  }

  void emitAlternativeEdges() {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *G) {
      for (const auto &Alternative : Node->alternatives()) {
        if (visited(std::make_pair(Node, Alternative), Skip)) {
          continue;
        }
        emitEdge(Node, Alternative, "color=green dir=none constraint=false");
        Skip.insert(std::make_pair<>(Alternative, Node));
        Skip.insert(std::make_pair<>(Node, Alternative));
      }
    }
  }

  void emitImplicationEdges() {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *G) {
      for (const auto &Implication : Node->implications()) {
        if (visited(std::make_pair(Node, Implication), Skip)) {
          continue;
        }
        if (std::find(Implication->implications_begin(),
                      Implication->implications_end(),
                      Node) != Implication->implications_end()) {
          emitEdge(Node, Implication, "color=blue dir=both constraint=false");
          Skip.insert(std::make_pair<>(Implication, Node));
        } else {
          emitEdge(Node, Implication, "color=blue constraint=false");
        }
        Skip.insert(std::make_pair<>(Node, Implication));
      }
    }
  }

  /// Output feature model (tree) recursively.
  ///
  /// \param[in] Node Root of subtree.
  /// \param[in] Indent Value to indent statements in dot file.
  void emitCluster(const NodeRef Node, const int Indent = 0) {
    O.indent(Indent);
    emitNode(Node);
    if (Node->children_begin() != Node->children_end()) {
      O.indent(Indent + 2) << "subgraph cluster_" << static_cast<void *>(Node)
                           << " {\n";
      O.indent(Indent + 4) << "label=\"\";\n";
      O.indent(Indent + 4) << "margin=0;\n";
      O.indent(Indent + 4) << "style=invis;\n";
      for (auto *Child : *Node) {
        emitCluster(Child, Indent + 2);
        O.indent(Indent + 2);
        emitEdge(Node, Child,
                 llvm::formatv("arrowhead={0}",
                               Child->isOptional() ? "odot" : "dot"));
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

  /// Output \a Node with custom attributes.
  void emitNode(const NodeRef Node) {
    std::string Label = llvm::formatv(
        "<<table align=\"center\" valign=\"middle\" border=\"0\" "
        "cellborder=\"0\" "
        "cellpadding=\"5\"><tr><td>{0}{1}</td></tr></"
        "table>>",
        DOT::EscapeString(Node->getName().str()),
        (Node->getFeatureSourceRange()
             ? "</td></tr><hr/><tr><td>" +
                   DOT::EscapeString(Node->getFeatureSourceRange()->toString())
             : ""));

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
