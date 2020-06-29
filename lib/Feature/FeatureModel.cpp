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
  std::string Key = Feature->getName();
  if (!Features.try_emplace(Key, std::move(Feature)).second) {
    return false;
  }
  auto *Value = Features[Key].get();
  for (auto *Child : Value->children()) {
    Child->setParent(Value);
  }
  if (Value->isRoot()) {
    if (*Root->getParent() == *Value) {
      Root = Value;
    } else {
      Value->setParent(Root);
    }
  } else {
    Value->getParent()->addChild(Value);
  }
  OrderedFeatures.insert(Value);
  return true;
}

bool FeatureModelBuilder::buildConstraints() {
  for (const auto &Feature : Features.keys()) {
    for (const auto &Exclude : Excludes[Feature]) {
      Features[Feature]->addExclude(Features[Exclude].get());
    }
  }

  for (const ConstraintTy &Clause : Constraints) {
    if (Clause.size() > 2) {
      // TODO(s9latimm): add missing constraint handling
      bool NoNegation = true;
      for (const auto &Literal : Clause) {
        NoNegation &= Literal.second;
      }
      if (NoNegation) {
        for (const auto &Literal : Clause) {
          for (const auto &OtherLiteral : Clause) {
            if (Literal.first != OtherLiteral.first) {
              Features[Literal.first]->addAlternative(
                  Features[OtherLiteral.first].get());
            }
          }
        }
      } else {
        llvm::errs() << "warning: Unrecognized clause.\n";
      }
    } else if (Clause.size() == 2) {
      if (Clause[0].second != Clause[1].second) {
        if (Clause[0].second) { // A || !B
          Features[Clause[1].first]->addImplication(
              Features[Clause[0].first].get());
        } else { // !A || B
          Features[Clause[0].first]->addImplication(
              Features[Clause[1].first].get());
        }
      } else if (!(Clause[0].second || Clause[1].second)) { // !A || !B
        Features[Clause[0].first]->addExclude(Features[Clause[1].first].get());
        Features[Clause[1].first]->addExclude(Features[Clause[0].first].get());
      } else if (Clause[0].second && Clause[1].second) { // A || B
        Features[Clause[0].first]->addAlternative(
            Features[Clause[1].first].get());
        Features[Clause[1].first]->addAlternative(
            Features[Clause[0].first].get());
      }
    } else if (Clause.size() == 1 && !Clause.front().second) {
      llvm::errs() << "error: Clause \'!" << Clause.front().first
                   << "\' invalidates feature.\n";
      return false;
    } else {
      llvm::errs() << "warning: Empty or trivial clause.\n";
    }
  }
  return true;
}

bool FeatureModelBuilder::buildTree(const string &Key,
                                    std::set<std::string> &Visited) {
  if (find(Visited.begin(), Visited.end(), Key) != Visited.end()) {
    llvm::errs() << "error: Cycle or duplicate edge in \'" << Key << "\'.\n";
    return false;
  }
  Visited.insert(Key);

  if (find(Features.keys().begin(), Features.keys().end(), Key) ==
      Features.keys().end()) {
    llvm::errs() << "error: Missing feature \'\'" << Key << "\'.\n";
    return false;
  }

  for (const auto &Child : Children[Key]) {
    if (!buildTree(Child, Visited)) {
      return false;
    }
    Features[Key]->addChild(Features[Child].get());
    Features[Child]->setParent(Features[Key].get());
  }
  return true;
}

FeatureModelBuilder *FeatureModelBuilder::setRoot(const std::string &RootKey) {
  assert(this->Root == nullptr && "Root already set.");

  if (Features.find(RootKey) == Features.end()) {
    addFeature(RootKey, false);
  }
  this->Root = Features[RootKey].get();

  for (const auto &Key : Features.keys()) {
    if (Key != RootKey && Parents.find(Key) == Parents.end()) {
      Children[RootKey].push_back(Key);
      Parents[Key] = RootKey;
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
  if (!buildTree(Root->getName(), Visited) || !buildConstraints()) {
    return nullptr;
  }
  return std::make_unique<FeatureModel>(Name, Path, std::move(Features), Root);
}

std::unique_ptr<FeatureModel> FeatureModelBuilder::buildSimpleFeatureModel(
    const std::vector<std::pair<std::string, std::string>> &B,
    const std::vector<std::pair<
        std::string, std::pair<std::string, NumericFeature::ValuesVariantType>>>
        &N) {
  init();
  for (const auto &Binary : B) {
    addFeature(Binary.first);
    addFeature(Binary.second);
    addParent(Binary.second, Binary.first);
  }
  for (const auto &Numeric : N) {
    addFeature(Numeric.first);
    addFeature(Numeric.second.first, Numeric.second.second);
    addParent(Numeric.second.first, Numeric.first);
  }
  return std::move(buildFeatureModel());
}
} // namespace vara::feature
