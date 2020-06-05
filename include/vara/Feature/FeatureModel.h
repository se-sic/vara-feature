#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Feature.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/GraphWriter.h"

#include <algorithm>
#include <utility>

template <> struct llvm::DenseMapInfo<std::string> {
  static inline string getEmptyKey() { return ""; }

  static inline string getTombstoneKey() { return ""; }

  static unsigned getHashValue(const std::string &PtrVal) {
    return std::hash<std::string>{}(PtrVal);
  }

  static bool isEqual(const string &LHS, const string &RHS) {
    return LHS == RHS;
  }
};

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureModel
//===----------------------------------------------------------------------===//

class FeatureModel {
public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<Feature>>;
  using FeatureListType = std::vector<Feature *>;
  using ConstraintTy = llvm::SmallVector<std::pair<std::string, bool>, 3>;
  using ConstraintsTy = std::vector<ConstraintTy>;

private:
  string Name;
  fs::path RootPath;
  FeatureMapTy FeatureMap;
  FeatureListType Features;
  ConstraintsTy Constraints;
  Feature *Root;

public:
  FeatureModel(string Name, fs::path RootPath, FeatureMapTy FeatureMap,
               ConstraintsTy Constraints, Feature *Root,
               FeatureListType Features)
      : Name(std::move(Name)), RootPath(std::move(RootPath)),
        FeatureMap(std::move(FeatureMap)), Constraints(std::move(Constraints)),
        Root(Root), Features(std::move(Features)) {}

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] fs::path getPath() const { return RootPath; }

  [[nodiscard]] Feature *getRoot() const {
    assert(Root);
    return Root;
  }

  [[nodiscard]] unsigned int size() { return FeatureMap.size(); }

  void view() { ViewGraph(this, "FeatureModel-" + this->getName()); }

  using ordered_feature_iterator = typename FeatureListType::iterator;
  using const_ordered_feature_iterator =
      typename FeatureListType::const_iterator;

  ordered_feature_iterator begin() { return Features.begin(); }
  [[nodiscard]] const_ordered_feature_iterator begin() const {
    return Features.begin();
  }

  ordered_feature_iterator end() { return Features.end(); }
  [[nodiscard]] const_ordered_feature_iterator end() const {
    return Features.end();
  }

  Feature *getFeature(llvm::StringRef Name) { return FeatureMap[Name].get(); }

  LLVM_DUMP_METHOD
  void dump() const;

  class Builder {
    using EdgeMapType =
        typename llvm::StringMap<llvm::SmallVector<std::string, 3>>;

    int Index;
    std::string VmName;
    fs::path RootPath;
    std::string Root;
    llvm::StringMap<Feature::Builder> FeatureBuilder;
    FeatureMapTy FeatureMap;
    FeatureListType Features;
    FeatureModel::ConstraintsTy Constraints;
    llvm::StringMap<std::string> Parents;
    EdgeMapType Children;
    EdgeMapType Excludes;
    EdgeMapType RawImplications;
    EdgeMapType Alternatives;

  public:
    Builder() : Index(0) {}

    bool addFeature(std::string Name, bool Opt,
                    std::variant<std::pair<int, int>, std::vector<int>> Values,
                    std::optional<FeatureSourceRange> Loc = std::nullopt) {
      return FeatureBuilder.try_emplace(Name, Name, Opt, Values, std::move(Loc))
          .second;
    }

    bool addFeature(std::string Name, bool Opt,
                    std::optional<FeatureSourceRange> Loc = std::nullopt) {
      return FeatureBuilder.try_emplace(Name, Name, Opt, std::move(Loc)).second;
    }

    void addChild(const std::string &P, const std::string &C) {
      Children[P].push_back(C);
      Parents[C] = P;
    }

    void addAlternative(const std::string &A, const std::string &B) {
      Alternatives[A].push_back(B);
      Alternatives[B].push_back(A);
    }

    void addExclude(const std::string &F, const std::string &E) {
      Excludes[F].push_back(E);
    }

    void addImplication(const std::string &A, const std::string &B) {
      RawImplications[A].push_back(B);
    }

    void addConstraint(const ConstraintTy &C) { Constraints.push_back(C); }

    void setVmName(std::string N) { this->VmName = std::move(N); }

    void setRootPath(fs::path P) { this->RootPath = std::move(P); }

    void setRoot(std::string R = "root");

    std::unique_ptr<FeatureModel> build();

  private:
    void buildConstraints();

    bool buildTree(const std::string &F);

    void buildFeatures();
  };
};
} // namespace vara::feature

inline std::ostream &operator<<(std::ostream &Out,
                                const vara::feature::Feature *Feature) {
  Feature->print(Out);
  return Out;
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Out,
                                     const vara::feature::Feature *Feature) {
  Feature->print(Out);
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
    // Output the header for the graph...
    writeHeader(Title);

    // Emit all of the nodes in the graph...
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
  /// \param Edge may be already visited.
  /// \param Skip contains existing edges.
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
      for (auto &Exclude : Node->excludes()) {
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
      for (auto &Alternative : Node->alternatives()) {
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
      for (auto &Implication : Node->implications()) {
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
  /// \param Node Root of subtree.
  /// \param Indent Value to indent statements in dot file.
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
        "cellpadding=\"5\"><tr><td>{0}</td></tr><hr/><tr><td>{1}{2}</td></tr></"
        "table>>",
        Node->getIndex(), DOT::EscapeString(Node->getName().str()),
        (Node->getLocation()
             ? "</td></tr><hr/><tr><td>" +
                   DOT::EscapeString(Node->getLocation()->toString())
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
