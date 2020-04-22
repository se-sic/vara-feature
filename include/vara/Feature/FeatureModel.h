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
  FeatureMapTy Features;
  ConstraintsTy Constraints;
  Feature *Root;

public:
  FeatureModel(string VM, FeatureMapTy &Features, ConstraintsTy &Constraints)
      : Name(std::move(VM)), Features(std::move(Features)),
        Constraints(std::move(Constraints)), Root(Features["root"].get()) {}

  [[nodiscard]] llvm::StringRef getName() const { return Name; }

  [[nodiscard]] Feature *getRoot() const { return Root; }

  [[nodiscard]] unsigned int size() { return Features.size(); }

  struct FeatureModelIter {
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

  static bool
  skip(std::pair<Feature *, Feature *> Edge,
       const llvm::SmallSet<std::pair<Feature *, Feature *>, 10> &Visited) {
    for (const auto &P : Visited) {
      if (P.first == Edge.first && P.second == Edge.second) {
        return true;
      }
    }
    return false;
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
    return "Feature model for " + FM->getName().str();
  }

  static std::string getNodeLabel(const vara::feature::Feature *Node,
                                  const vara::feature::FeatureModel *FM) {
    std::stringstream S;
    S << Node->getName();
    return S.str();
  }

  static std::string getNodeAttributes(const vara::feature::Feature *Node,
                                       const vara::feature::FeatureModel *FM) {
    return "";
  }

  static std::string
  getEdgeAttributes(const vara::feature::Feature *Node,
                    vara::feature::Feature::feature_iterator I,
                    const vara::feature::FeatureModel *FM) {
    std::stringstream S;
    S << "arrowhead=\"";
    if ((*I)->isOptional()) {
      S << 'o';
    }
    S << "dot\" ";
    return S.str();
  }

  template <typename GraphWriter>
  static void addCustomGraphFeatures(vara::feature::FeatureModel *FM,
                                     GraphWriter &W) {
    using FeatureSetTy = llvm::SmallSet<
        std::pair<vara::feature::Feature *, vara::feature::Feature *>, 10>;
    FeatureSetTy SkipE;
    FeatureSetTy SkipI;
    FeatureSetTy SkipA;
    for (auto *Node : *FM) {
      for (auto &Exclude : Node->excludes()) {
        if (vara::feature::FeatureModel::skip(std::make_pair(Node, Exclude),
                                              SkipE)) {
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
        if (vara::feature::FeatureModel::skip(std::make_pair(Node, Implication),
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
        if (vara::feature::FeatureModel::skip(std::make_pair(Node, Alternative),
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
