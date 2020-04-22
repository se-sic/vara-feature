#ifndef VARA_FEATURE_FEATUREMODEL_H
#define VARA_FEATURE_FEATUREMODEL_H

#include "vara/Feature/Feature.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/Support/GraphWriter.h"

using std::string;

namespace vara {

//===----------------------------------------------------------------------===//
//                               FeatureModel
//===----------------------------------------------------------------------===//

class FeatureModel {
public:
  using FeatureMapTy = llvm::StringMap<std::unique_ptr<FMFeature>>;
  using ConstraintsTy = std::vector<std::vector<std::pair<FMFeature *, bool>>>;

private:
  string Name;
  FeatureMapTy Features;
  ConstraintsTy Constraints;
  FMFeature *Root;

public:
  FeatureModel(string VM, FeatureMapTy &Features, ConstraintsTy &Constraints)
      : Name(std::move(VM)), Features(std::move(Features)),
        Constraints(std::move(Constraints)), Root(Features["root"].get()) {}

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] FMFeature *getRoot() const { return Root; }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  struct FeatureModelIter {
    FeatureMapTy::iterator It;

    explicit FeatureModelIter(FeatureMapTy::iterator It) : It(It) {}

    FMFeature *operator*() const { return It->second.get(); }

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

  static bool
  skip(std::pair<FMFeature *, FMFeature *> Edge,
       const llvm::SmallSet<std::pair<FMFeature *, FMFeature *>, 10> &Visited) {
    for (const auto &P : Visited) {
      if (P.first == Edge.first && P.second == Edge.second) {
        return true;
      }
    }
    return false;
  }
};
} // namespace vara

inline std::ostream &operator<<(std::ostream &Out,
                                const vara::FMFeature *FMFeature) {
  FMFeature->print(Out);
  return Out;
}

inline llvm::raw_ostream &operator<<(llvm::raw_ostream &Out,
                                     const vara::FMFeature *FMFeature) {
  FMFeature->print(Out);
  return Out;
}

namespace llvm {

//===----------------------------------------------------------------------===//
//                     (Dot)GraphTraits for FeatureModel
//===----------------------------------------------------------------------===//

template <> struct GraphTraits<vara::FeatureModel *> {
  using NodeRef = typename vara::FMFeature *;

  static NodeRef getEntryNode(const vara::FeatureModel *FM) {
    return FM->getRoot();
  }

  using nodes_iterator = typename vara::FeatureModel::FeatureModelIter;

  static nodes_iterator nodes_begin(vara::FeatureModel *FM) {
    return FM->begin();
  }

  static nodes_iterator nodes_end(vara::FeatureModel *FM) { return FM->end(); }

  using ChildIteratorType = typename vara::FMFeature::feature_iterator;

  static ChildIteratorType child_begin(NodeRef N) { return N->begin(); }

  static ChildIteratorType child_end(NodeRef N) { return N->end(); }

  static size_t size(vara::FeatureModel *FM) { return FM->size(); }
};

template <>
struct DOTGraphTraits<vara::FeatureModel *> : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool IsSimple = false)
      : DefaultDOTGraphTraits(IsSimple) {}

  static std::string getGraphName(const vara::FeatureModel *FM) {
    return "Feature model for " + FM->getName().str();
  }

  std::string getNodeLabel(const vara::FMFeature *Node,
                           const vara::FeatureModel *FM) {
    std::stringstream S;
    S << Node->getName();
    return S.str();
  }

  static std::string getNodeAttributes(const vara::FMFeature *Node,
                                       const vara::FeatureModel *FM) {
    return "";
  }

  std::string getEdgeAttributes(const vara::FMFeature *Node,
                                vara::FMFeature::feature_iterator I,
                                const vara::FeatureModel *FM) {
    std::stringstream S;
    S << "arrowhead=\"";
    if ((*I)->isOptional()) {
      S << 'o';
    }
    S << "dot\" ";
    return S.str();
  }

  template <typename GraphWriter>
  static void addCustomGraphFeatures(vara::FeatureModel *FM, GraphWriter &W) {
    llvm::SmallSet<std::pair<vara::FMFeature *, vara::FMFeature *>, 10> SkipE,
        SkipI, SkipA;
    for (auto *Node : *FM) {
      for (auto &Exclude : Node->excludes()) {
        if (vara::FeatureModel::skip(std::make_pair(Node, Exclude), SkipE)) {
          continue;
        }
        if (std::find(Exclude->excludes_begin(), Exclude->excludes_end(),
                      Node) != Exclude->excludes_end()) {
          W.emitEdge(Node, -1, Exclude, -1, "color=red dir=both");
          SkipE.insert(std::make_pair<>(Exclude, Node));
        } else {
          W.emitEdge(Node, 0, Exclude, 0, "color=red");
        }
        SkipE.insert(std::make_pair<>(Node, Exclude));
      }
      for (auto &Implication : Node->implications()) {
        if (vara::FeatureModel::skip(std::make_pair(Node, Implication),
                                     SkipI)) {
          continue;
        }
        if (std::find(Implication->implications_begin(),
                      Implication->implications_end(),
                      Node) != Implication->implications_end()) {
          W.emitEdge(Node, -1, Implication, -1, "color=blue dir=both");
          SkipI.insert(std::make_pair<>(Implication, Node));
        } else {
          W.emitEdge(Node, 0, Implication, 0, "color=blue");
        }
        SkipI.insert(std::make_pair<>(Node, Implication));
      }
      for (auto &Alternative : Node->alternatives()) {
        if (vara::FeatureModel::skip(std::make_pair(Node, Alternative),
                                     SkipA)) {
          continue;
        }
        W.emitEdge(Node, -1, Alternative, -1, "color=green dir=none");
        SkipA.insert(std::make_pair<>(Alternative, Node));
        SkipA.insert(std::make_pair<>(Node, Alternative));
      }
    }
  }
};
} // namespace llvm

#endif // VARA_FEATURE_FEATUREMODEL_H
