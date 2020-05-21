#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Feature.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/GraphWriter.h"

using std::string;

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               FeatureModel
//===----------------------------------------------------------------------===//

class FeatureModel {
public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<Feature>>;
  using ConstraintsTy = std::vector<std::vector<std::pair<Feature *, bool>>>;

private:
  string Name;
  fs::path RootPath;
  FeatureMapTy Features;
  ConstraintsTy Constraints;
  Feature *Root;

public:
  FeatureModel(string Name, fs::path RootPath, FeatureMapTy Features,
               ConstraintsTy Constraints)
      : Name(std::move(Name)), RootPath(std::move(RootPath)),
        Features(std::move(Features)), Constraints(std::move(Constraints)),
        Root(this->Features["root"].get()) {}

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] fs::path getPath() const { return RootPath; }

  [[nodiscard]] Feature *getRoot() const {
    assert(Root);
    return Root;
  }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  struct FeatureModelIter : std::iterator<std::forward_iterator_tag, Feature &,
                                          ptrdiff_t, Feature *, Feature &> {
    FeatureMapTy::iterator It;

    explicit FeatureModelIter(FeatureMapTy::iterator It) : It(It) {}

    Feature *operator*() const { return It->second.get(); }

    FeatureModelIter &operator++() {
      It++;
      return *this;
    }

    bool operator==(const FeatureModelIter &Other) const {
      return It == Other.It;
    }

    bool operator!=(const FeatureModelIter &Other) const {
      return not operator==(Other);
    }
  };

  FeatureModelIter begin() { return FeatureModelIter(Features.begin()); }

  FeatureModelIter end() { return FeatureModelIter(Features.end()); }

  LLVM_DUMP_METHOD
  void dump() const {
    for (const auto &F : this->Features) {
      F.second->dump();
      llvm::outs() << "\n";
    }
    for (auto C : this->Constraints) {
      for (auto F : C) {
        if (!F.second) {
          llvm::outs() << "!";
        }
        llvm::outs() << F.first->getName();
        if (F != C.back()) {
          llvm::outs() << " | ";
        }
      }
      llvm::outs() << "\n";
    }
  }
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
    std::string GraphName = "Feature model for " + G->getName().str() + "\n" +
                            G->getPath().string();

    if (!Title.empty()) {
      O << "digraph \"" << DOT::EscapeString(Title) << "\" {\n";
    } else {
      O << "digraph graph_" << static_cast<void *>(G) << " {\n";
    }

    O << "\tgraph [pad=.5,nodesep=2,ranksep=2,splines=true,newrank=true,"
         "fontname=cmu,bgcolor=white,rankdir=tb,overlap=false,label=\""
      << DOT::EscapeString(GraphName) << "\"];\n\n";
  }

  void writeNodes() {
    emitClusterRecursively(G->getRoot());
    emitExcludeEdges();
    emitImplicationEdges();
    emitAlternativeEdges();
  }

  void writeFooter() { O << "}\n"; }

  using FeatureEdgeSetTy = llvm::SmallSet<
      std::pair<vara::feature::Feature *, vara::feature::Feature *>, 10>;

  /// Checks whether an edge would be a duplicate.
  ///
  /// \param Skip Contains existing edges.
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
          emitEdge(Node, Exclude, "color=red,dir=both,constraint=false");
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
        emitEdge(Node, Alternative, "color=green,dir=none,constraint=false");
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
          emitEdge(Node, Implication, "color=blue,dir=both,constraint=false");
          Skip.insert(std::make_pair<>(Implication, Node));
        } else {
          emitEdge(Node, Implication, "color=blue,constraint=false");
        }
        Skip.insert(std::make_pair<>(Node, Implication));
      }
    }
  }

  void emitClusterRecursively(const NodeRef Node, const int Indent = 0) {
    std::string I = std::string(Indent, '\t');

    O << I;
    emitNode(Node);

    if (Node->begin() != Node->end()) {
      O << I << "\tsubgraph cluster_" << static_cast<void *>(Node) << " {\n"
        << I << "\t\tlabel=\"\";\n"
        << I << "\t\tmargin=0;\n"
        << I << "\t\tstyle=invis;\n";
      for (auto *Child : *Node) {
        emitClusterRecursively(Child, Indent + 1);
        O << I << '\t';
        emitEdge(Node, Child);
      }
      O << I << "\t\t{\n" << I << "\t\trank=same;\n";
      for (auto *Child : *Node) {
        O << I << "\t\t\tnode_" << static_cast<void *>(Child) << ";\n";
      }
      O << I << "\t\t}\n" << I << "\t}\n";
    }
  }

  void emitNode(const NodeRef Node) {
    std::string NodeAttributes =
        "shape=box,margin=.1,fontsize=12,fontname=\"CMU Typewriter\",";

    std::string Label =
        "<<table align=\"center\" valign=\"middle\" "
        "border=\"0\" cellborder=\"0\" cellpadding=\"5\"><tr><td>" +
        Node->getName() +
        (Node->getLocation()
             ? "</td></tr><hr/><tr><td>" + Node->getLocation()->toString()
             : "") +
        "</td></tr></table>>";

    O << "\tnode_" << static_cast<void *>(Node) << " [" << NodeAttributes
      << "label=" << Label << "];\n";
  }

  void emitEdge(const NodeRef SrcNode, const NodeRef DestNode,
                const std::string &Attrs = "") {
    O << "\tnode_" << static_cast<void *>(SrcNode) << " -> node_"
      << static_cast<void *>(DestNode);
    if (!Attrs.empty()) {
      O << " [" << Attrs << "]";
    }
    O << ";\n";
  }
};

} // namespace llvm

#endif // VARA_FEATURE_FEATUREMODEL_H
