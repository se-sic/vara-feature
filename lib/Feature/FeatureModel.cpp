#include "vara/Feature/FeatureModel.h"

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

bool FeatureModelBuilder::buildConstraints() {
  auto B = BuilderVisitor(this);
  for (const auto &C : Constraints) {
    C->accept(B);
  }
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
        } else {
          Skip.insert(Child);
        }
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

  addRelationship(Relationship::RelationshipKind::RK_OR, {"AA", "AB"}, "A");

  if (!buildTree(std::string(Root->getName()), Visited) ||
      !buildConstraints()) {
    return nullptr;
  }
  return std::make_unique<FeatureModel>(Name, Path, std::move(Features),
                                        std::move(Constraints),
                                        std::move(Relationships), Root);
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildSimpleFeatureModel(
    const std::vector<std::pair<std::string, std::string>> &B,
    const std::vector<std::pair<
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
  return std::move(buildFeatureModel());
}
bool FeatureModelBuilder::addFeature(Feature &F) {
  std::optional<FeatureSourceRange> Loc =
      F.getFeatureSourceRange()
          ? std::make_optional(FeatureSourceRange(*F.getFeatureSourceRange()))
          : std::nullopt;
  switch (F.getKind()) {
  case Feature::FeatureKind::FK_BINARY:
    if (!makeFeature<BinaryFeature>(std::string(F.getName()), F.isOptional(),
                                    Loc)) {
      return false;
    }
    break;
  // locale var -> assert
  case Feature::FeatureKind::FK_NUMERIC:
    if (!makeFeature<NumericFeature>(
            std::string(F.getName()),
            llvm::dyn_cast<NumericFeature>(&F)->getValues(), F.isOptional(),
            Loc)) {
      return false;
    }
    break;
  default:
    return false;
  }
  if (!F.isRoot()) {
    this->addParent(std::string(F.getName()),
                    std::string(F.getParentFeature()->getName()));
  }
  for (const auto *Child : F.children()) {
    const auto *C = llvm::dyn_cast<vara::feature::Feature>(Child);
    if (C) {
      this->addParent(std::string(C->getName()), std::string(F.getName()));
    }
  }
  //  for (const auto *Exclude : F.excludes()) {
  //    this->addExclude(std::string(F.getName()),
  //    std::string(Exclude->getName()));
  //  }
  //  for (const auto *Alternative : F.alternatives()) {
  //    this->addConstraint({{std::string(F.getName()), true},
  //                         {std::string(Alternative->getName()), true}});
  //  }
  //  for (const auto *Implication : F.implications()) {
  //    this->addConstraint({{std::string(F.getName()), false},
  //                         {std::string(Implication->getName()), true}});
  //  }
  return true;
}
} // namespace vara::feature
