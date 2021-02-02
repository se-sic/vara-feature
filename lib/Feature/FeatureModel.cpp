#include "vara/Feature/FeatureModel.h"

#include <algorithm>

namespace vara::feature {
void FeatureModel::dump() const {
  for (const auto &Feature : Features) {
    llvm::outs() << "{\n";
    Feature.second->dump();
    llvm::outs() << "}\n";
  }
  llvm::outs() << '\n';
}

bool FeatureModel::addFeature(std::unique_ptr<Feature> Feature) {
  // TODO(s9latimm): check consistency
  auto FeatureName = std::string(Feature->getName());
  if (!Features.try_emplace(FeatureName, std::move(Feature)).second) {
    return false;
  }
  auto *Value = Features[FeatureName].get();
  for (auto *Child : Value->children()) {
    Child->setParent(*Value);
  }
  if (Value->isRoot()) {
    if (*Root->getParentFeature() == *Value) {
      Root = Value;
    } else {
      Value->setParent(Root);
    }
  } else {
    Value->getParent()->addEdge(Value);
  }
  OrderedFeatures.insert(Value);
  return true;
}

/// Decide whether two features are mutual exclusive. Beware that this method
/// only detects very simple trees with binary excludes.
bool isSimpleMutex(const Feature *A, const Feature *B) {
  return std::any_of(
      A->excludes().begin(), A->excludes().end(), [A, B](const auto *E) {
        if (const auto *LHS =
                llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand());
            LHS) {
          if (const auto *RHS = llvm::dyn_cast<PrimaryFeatureConstraint>(
                  E->getRightOperand());
              RHS) {
            // A excludes B
            if (LHS->getFeature() &&
                LHS->getFeature()->getName() == A->getName() &&
                RHS->getFeature() &&
                RHS->getFeature()->getName() == B->getName()) {
              return std::any_of(
                  B->excludes().begin(), B->excludes().end(),
                  [A, B](const auto *E) {
                    if (const auto *LHS =
                            llvm::dyn_cast<PrimaryFeatureConstraint>(
                                E->getLeftOperand());
                        LHS) {
                      if (const auto *RHS =
                              llvm::dyn_cast<PrimaryFeatureConstraint>(
                                  E->getRightOperand());
                          RHS) {
                        // B excludes A
                        return LHS->getFeature() &&
                               LHS->getFeature()->getName() == B->getName() &&
                               RHS->getFeature() &&
                               RHS->getFeature()->getName() == A->getName();
                      }
                    }
                    return false;
                  });
            }
          }
        }
        return false;
      });
}

/// Collect mutual exclusive feature constraints for clean up.
llvm::SmallSet<Constraint *, 3>
cleanUpMutualExclusiveConstraints(const Feature *A,
                                  const llvm::SmallSet<Feature *, 3> &Xor) {
  llvm::SmallSet<Constraint *, 3> Remove;
  for (const auto *E : A->excludes()) {
    if (auto *LHS =
            llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand());
        LHS) {
      if (auto *RHS =
              llvm::dyn_cast<PrimaryFeatureConstraint>(E->getRightOperand());
          RHS) {
        if (LHS->getFeature() && LHS->getFeature()->getName() == A->getName() &&
            RHS->getFeature() && Xor.count(RHS->getFeature())) {
          Remove.insert(LHS);
        } else if (LHS->getFeature() && Xor.count(LHS->getFeature()) &&
                   RHS->getFeature() &&
                   RHS->getFeature()->getName() == A->getName()) {
          Remove.insert(RHS);
        }
      }
    }
  }
  return Remove;
}

void FeatureModelBuilder::detectXMLAlternatives() {
  for (const auto &FeatureName : Features.keys()) {
    std::vector<std::string> Frontier(Children[FeatureName].begin(),
                                      Children[FeatureName].end());
    while (!Frontier.empty()) {
      const auto FName = std::string(Frontier.back());
      Frontier.pop_back();
      if (auto *F = llvm::dyn_cast<Feature>(Features[FName].get());
          F && !F->isOptional()) {
        llvm::SmallSet<Feature *, 3> Xor;
        Xor.insert(F);
        for (const auto &Name : Frontier) {
          if (auto *E =
                  llvm::dyn_cast<Feature>(Features[Frontier.back()].get());
              E && !E->isOptional()) {
            if (std::all_of(Xor.begin(), Xor.end(), [E](const auto *F) {
                  return isSimpleMutex(F, E);
                })) {
              Xor.insert(E);
            }
          }
        }
        if (Xor.size() > 1) {
          std::vector<std::string> V;
          for (auto *E : Xor) {
            Frontier.erase(std::remove(Frontier.begin(), Frontier.end(),
                                       E->getName().str()),
                           Frontier.end());
            V.push_back(E->getName().str());
            for (auto *R : cleanUpMutualExclusiveConstraints(E, Xor)) {
              E->removeConstraintNonPreserve(R);
            }
          }
          emplaceRelationship(Relationship::RelationshipKind::RK_ALTERNATIVE, V,
                              FeatureName.str());
        }
      }
    }
  }
}

bool FeatureModelBuilder::buildConstraints() {
  auto B = BuilderVisitor(this);
  for (const auto &C : Constraints) {
    C->accept(B);
  }
  detectXMLAlternatives();
  return true;
}

bool FeatureModelBuilder::buildTree(const string &FeatureName,
                                    std::set<std::string> &Visited) {
  if (find(Visited.begin(), Visited.end(), FeatureName) != Visited.end()) {
    llvm::errs() << "error: Cycle or duplicate edge in \'" << FeatureName
                 << "\'.\n";
    return false;
  }
  Visited.insert(FeatureName);

  if (find(Features.keys().begin(), Features.keys().end(), FeatureName) ==
      Features.keys().end()) {
    llvm::errs() << "error: Missing feature \'\'" << FeatureName << "\'.\n";
    return false;
  }

  for (const auto &Child : Children[FeatureName]) {
    if (Parents[Child] != FeatureName) {
      llvm::errs() << "error: Parent of \'" << Child << "\' does not match \'"
                   << FeatureName << "\'.\n";
      return false;
    }
    if (!buildTree(Child, Visited)) {
      return false;
    }
  }

  llvm::SmallSet<std::string, 3> Skip;
  if (RelationshipEdges.find(FeatureName) != RelationshipEdges.end()) {
    for (const auto &Pair : RelationshipEdges[FeatureName]) {
      auto R = std::make_unique<Relationship>(Pair.first);
      Features[FeatureName]->addEdge(R.get());
      R->setParent(Features[FeatureName].get());
      for (const auto &Child : Pair.second) {
        if (Children[FeatureName].count(Child) == 0) {
          llvm::errs() << "error: Related node \'" << Child
                       << "\' is not child of \'" << FeatureName << "\'.\n";
          return false;
        }
        Skip.insert(Child);
        R->addEdge(Features[Child].get());
        Features[Child]->setParent(R.get());
      }
      Relationships.push_back(std::move(R));
    }
  }
  for (const auto &Child : Children[FeatureName]) {
    if (Skip.count(Child) > 0) {
      continue;
    }
    Features[FeatureName]->addEdge(Features[Child].get());
    Features[Child]->setParent(Features[FeatureName].get());
  }
  return true;
}

FeatureModelBuilder *FeatureModelBuilder::setRoot(const std::string &RootName) {
  assert(this->Root == nullptr && "Root already set.");

  if (Features.find(RootName) == Features.end()) {
    makeFeature<BinaryFeature>(RootName, false);
  }
  this->Root = Features[RootName].get();

  for (const auto &FeatureName : Features.keys()) {
    if (FeatureName != RootName && Parents.find(FeatureName) == Parents.end()) {
      Children[RootName].insert(std::string(FeatureName));
      Parents[FeatureName] = RootName;
    }
  }
  return this;
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildFeatureModel() {
  if (!Root) {
    setRoot();
  }
  assert(Root && "Root not set.");
  std::set<std::string> Visited;

  if (!buildConstraints() ||
      !buildTree(std::string(Root->getName()), Visited)) {
    return nullptr;
  }
  return std::make_unique<FeatureModel>(Name, Path, std::move(Features),
                                        std::move(Constraints),
                                        std::move(Relationships), Root);
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildSimpleFeatureModel(
    const std::initializer_list<std::pair<std::string, std::string>> &B,
    const std::initializer_list<std::pair<
        std::string, std::pair<std::string, NumericFeature::ValuesVariantType>>>
        &N) {
  init();
  for (const auto &Binary : B) {
    makeFeature<BinaryFeature>(Binary.first);
    makeFeature<BinaryFeature>(Binary.second);
    addParent(Binary.second, Binary.first);
  }
  for (const auto &Numeric : N) {
    makeFeature<BinaryFeature>(Numeric.first);
    makeFeature<NumericFeature>(Numeric.second.first, Numeric.second.second);
    addParent(Numeric.second.first, Numeric.first);
  }
  return buildFeatureModel();
}
bool FeatureModelBuilder::addFeature(Feature &F) {
  std::optional<FeatureSourceRange> Loc =
      F.getFeatureSourceRange()
          ? std::make_optional(FeatureSourceRange(*F.getFeatureSourceRange()))
          : std::nullopt;
  switch (F.getKind()) {
  case Feature::FeatureKind::FK_BINARY: {
    if (auto *BF = llvm::dyn_cast<BinaryFeature>(&F); BF) {
      if (!makeFeature<BinaryFeature>(std::string(BF->getName()),
                                      BF->isOptional(), Loc)) {
        return false;
      }
    } else {
      llvm::errs() << "error: Could not cast \'" << F.getName()
                   << "\' to \'BinaryFeature\'.\n";
      return false;
    }
    break;
  }
  case Feature::FeatureKind::FK_NUMERIC: {
    if (auto *NF = llvm::dyn_cast<NumericFeature>(&F); NF) {
      if (!makeFeature<NumericFeature>(std::string(NF->getName()),
                                       NF->getValues(), NF->isOptional(),
                                       Loc)) {
        return false;
      }
    } else {
      llvm::errs() << "error: Could not cast \'" << F.getName()
                   << "\' to \'NumericFeature\'.\n";
      return false;
    }
    break;
  }
  default:
    return false;
  }
  if (!F.isRoot()) {
    this->addParent(std::string(F.getName()),
                    std::string(F.getParentFeature()->getName()));
  }
  for (const auto *Child : F.children()) {
    if (const auto *C = llvm::dyn_cast<vara::feature::Feature>(Child); C) {
      this->addParent(std::string(C->getName()), std::string(F.getName()));
    }
  }
  return true;
}
} // namespace vara::feature
