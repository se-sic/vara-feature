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
//                     (Dot)GraphTraits for FeatureModel
//===----------------------------------------------------------------------===//

template <> struct GraphTraits<vara::feature::FeatureModel *> {
  using NodeRef = typename vara::feature::Feature *;

  static NodeRef getEntryNode(const vara::feature::FeatureModel *FM) {
    return FM->getRoot();
  }

  using nodes_iterator = typename vara::feature::FeatureModel::FeatureModelIter;

  static nodes_iterator nodes_begin(vara::feature::FeatureModel *FM) {
    return FM->begin();
  }

  static nodes_iterator nodes_end(vara::feature::FeatureModel *FM) {
    return FM->end();
  }

  using ChildIteratorType = typename vara::feature::Feature::feature_iterator;

  static ChildIteratorType child_begin(NodeRef N) { return N->begin(); }

  static ChildIteratorType child_end(NodeRef N) { return N->end(); }

  static size_t size(vara::feature::FeatureModel *FM) { return FM->size(); }
};

template <>
struct DOTGraphTraits<vara::feature::FeatureModel *>
    : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool IsSimple = false)
      : DefaultDOTGraphTraits(IsSimple) {}

  static std::string getGraphName(const vara::feature::FeatureModel *FM) {
    return "Feature model for " + FM->getName().str() + "\n" +
           FM->getPath().string();
  }

  static std::string getGraphProperties(const vara::feature::FeatureModel *FM) {
    return "\tgraph[pad=0.5,nodesep=2,ranksep=2,splines=true,newrank=true,"
           "fontname=\"CMU Typewriter\"]";
  }

  static std::string
  getEdgeAttributes(const vara::feature::Feature *Node,
                    vara::feature::Feature::feature_iterator I,
                    const vara::feature::FeatureModel *FM) {
    return std::string("arrowhead=")
        .append((*I)->isOptional() ? "odot" : "dot");
  }

  /// Clusters nodes into subgraphs to enforce hierarchical ordering.
  ///
  /// \param L Indentation level (incremented recursively).
  static void addCustomGraphCluster(llvm::raw_ostream &O,
                                    vara::feature::Feature *Node, int L = 1) {
    std::string Indent = std::string(L, '\t');
    O << Indent << "Node" << Node
      << "[shape=box,margin=.1,fontsize=12,fontname=\"CMU "
         "Typewriter\",label=<<table align=\"center\" valign=\"middle\" "
         "border=\"0\" cellborder=\"0\" cellpadding=\"5\"><tr><td>"
      << Node->getName()
      << (Node->getLocation()
              ? "</td></tr><hr/><tr><td>" + Node->getLocation()->toString()
              : "")
      << "</td></tr></table>>];\n";
    if (Node->begin() != Node->end()) {
      O << Indent << "subgraph cluster_" << Node << " {\n"
        << Indent << "\tlabel=\"\";\n"
        << Indent << "\tmargin=0;\n"
        << Indent << "\tstyle=invis;\n";
      for (auto *C : *Node) {
        addCustomGraphCluster(O, C, L + 1);
      }
      O << Indent << "\t{\n" << Indent << "\t\trank=same;\n";
      for (auto *C : *Node) {
        O << Indent << "\t\tNode" << C << ";\n";
      }
      O << Indent << "\t}\n" << Indent << "}\n";
    }
  }

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

  template <typename GraphWriter>
  static void addCustomExcludeEdges(vara::feature::FeatureModel *FM,
                                    GraphWriter &W) {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *FM) {
      for (auto &Exclude : Node->excludes()) {
        if (visited(std::make_pair(Node, Exclude), Skip)) {
          continue;
        }
        if (std::find(Exclude->excludes_begin(), Exclude->excludes_end(),
                      Node) != Exclude->excludes_end()) {
          W.emitEdge(Node, -1, Exclude, -1,
                     "color=red,dir=both,constraint=false");
          Skip.insert(std::make_pair<>(Exclude, Node));
        } else {
          W.emitEdge(Node, -1, Exclude, -1, "color=red");
        }
        Skip.insert(std::make_pair<>(Node, Exclude));
      }
    }
  }

  template <typename GraphWriter>
  static void addCustomAlternativeEdges(vara::feature::FeatureModel *FM,
                                        GraphWriter &W) {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *FM) {
      for (auto &Alternative : Node->alternatives()) {
        if (visited(std::make_pair(Node, Alternative), Skip)) {
          continue;
        }
        W.emitEdge(Node, -1, Alternative, -1,
                   "color=green,dir=none,constraint=false");
        Skip.insert(std::make_pair<>(Alternative, Node));
        Skip.insert(std::make_pair<>(Node, Alternative));
      }
    }
  }

  template <typename GraphWriter>
  static void addCustomImplicationEdges(vara::feature::FeatureModel *FM,
                                        GraphWriter &W) {
    FeatureEdgeSetTy Skip;
    for (auto *Node : *FM) {
      for (auto &Implication : Node->implications()) {
        if (visited(std::make_pair(Node, Implication), Skip)) {
          continue;
        }
        if (std::find(Implication->implications_begin(),
                      Implication->implications_end(),
                      Node) != Implication->implications_end()) {
          W.emitEdge(Node, -1, Implication, -1,
                     "color=blue,dir=both,constraint=false");
          Skip.insert(std::make_pair<>(Implication, Node));
        } else {
          W.emitEdge(Node, -1, Implication, -1, "color=blue,constraint=false");
        }
        Skip.insert(std::make_pair<>(Node, Implication));
      }
    }
  }

  template <typename GraphWriter>
  static void addCustomGraphFeatures(vara::feature::FeatureModel *FM,
                                     GraphWriter &W) {
    addCustomExcludeEdges(FM, W);
    addCustomImplicationEdges(FM, W);
    addCustomAlternativeEdges(FM, W);
    addCustomGraphCluster(W.getOStream(), FM->getRoot());
  }
};
} // namespace llvm

#endif // VARA_FEATURE_FEATUREMODEL_H
