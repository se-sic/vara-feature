#include "vara/Feature/FeatureModel.h"

namespace vara::feature {
void FeatureModel::dump() const {
  for (const auto &F : Features) {
    llvm::outs() << "{\n";
    F.second->dump();
    llvm::outs() << "}\n";
  }
  llvm::outs() << '\n';
  for (auto C : this->Constraints) {
    for (const auto &F : C) {
      if (!F.second) {
        llvm::outs() << "!";
      }
      llvm::outs() << F.first;
      if (F != C.back()) {
        llvm::outs() << " | ";
      }
    }
    llvm::outs() << "\n";
  }
}

void FeatureModel::FeatureModelBuilder::buildConstraints() {
  for (const auto &F : Features.keys()) {
    for (const auto &E : Excludes[F]) {
      Features[F]->addExclude(Features[E].get());
    }
  }

  for (const auto &C : Constraints) {
    if (C.size() == 2) {
      if (C[0].second != C[1].second) {
        if (C[0].second) { // A || !B
          Features[C[1].first]->addImplication(Features[C[0].first].get());
        } else { // !A || B
          Features[C[0].first]->addImplication(Features[C[1].first].get());
        }
      } else if (!(C[0].second || C[1].second)) { // !A || !B
        Features[C[0].first]->addExclude(Features[C[1].first].get());
        Features[C[1].first]->addExclude(Features[C[0].first].get());
      } else if (C[0].second && C[1].second) { // A || B
        Features[C[0].first]->addAlternative(Features[C[1].first].get());
        Features[C[1].first]->addAlternative(Features[C[0].first].get());
      }
    } else if (C.size() > 2) {
      bool B = true;
      for (const auto &P : C) {
        B &= P.second;
      }
      if (B) {
        for (const auto &P : C) {
          for (const auto &PP : C) {
            if (P.first != PP.first) {
              Features[P.first]->addAlternative(Features[PP.first].get());
            }
          }
        }
      }
    }
  }
}

bool FeatureModel::FeatureModelBuilder::buildTree(
    const string &F, std::set<std::string> &Visited) {
  if (find(Visited.begin(), Visited.end(), F) != Visited.end()) {
    llvm::errs() << "error: Cycle or duplicate edge in \'" << F << "\'.\n";
    return false;
  }
  Visited.insert(F);

  if (find(Features.keys().begin(), Features.keys().end(), F) ==
      Features.keys().end()) {
    llvm::errs() << "error: Missing feature \'\'" << F << "\'.\n";
    return false;
  }

  for (const auto &C : Children[F]) {
    if (!buildTree(C, Visited)) {
      return false;
    }
    Features[F]->addChild(Features[C].get());
    Features[C]->setParent(Features[F].get());
  }
  return true;
}

FeatureModel::FeatureModelBuilder *
FeatureModel::FeatureModelBuilder::setRoot(const std::string &R) {
  assert(this->Root == nullptr && "Root already set.");

  if (Features.find(R) == Features.end()) {
    addFeature(R, false);
  }
  this->Root = Features[R].get();

  for (const auto &F : Features.keys()) {
    if (F != R && Parents.find(F) == Parents.end()) {
      Children[R].push_back(F);
      Parents[F] = R;
    }
  }
  return this;
}

std::unique_ptr<FeatureModel>
FeatureModel::FeatureModelBuilder::buildFeatureModel() {
  if (!Root) {
    setRoot();
  }
  assert(Root && "Root not set.");
  std::set<std::string> Visited;
  if (!buildTree(Root->getName(), Visited)) {
    return nullptr;
  }
  buildConstraints();
  return std::make_unique<FeatureModel>(VmName, Path, std::move(Features),
                                        Constraints, Root);
}

std::unique_ptr<FeatureModel>
FeatureModel::FeatureModelBuilder::buildSimpleFeatureModel(
    const std::vector<std::pair<std::string, std::string>> &B,
    const std::vector<std::pair<
        std::string, std::pair<std::string, NumericFeature::ValuesVariantType>>>
        &N) {
  init();
  for (const auto &P : B) {
    addFeature(P.first);
    addFeature(P.second);
    addChild(P.first, P.second);
  }
  for (const auto &P : N) {
    addFeature(P.first);
    addFeature(P.second.first, P.second.second);
    addChild(P.first, P.second.first);
  }
  return std::move(buildFeatureModel());
}
} // namespace vara::feature
