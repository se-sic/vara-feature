#include "vara/Feature/FeatureModel.h"

namespace vara::feature {
void FeatureModel::dump() const {
  for (const auto &F : Features) {
    llvm::outs() << "{\n";
    F->dump();
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

void FeatureModel::Builder::buildConstraints() {
  for (const auto &F : FeatureBuilder.keys()) {
    for (const auto &E : Excludes[F]) {
      FeatureBuilder[F].addExclude(FeatureBuilder[E].get());
    }
  }

  for (const auto &C : Constraints) {
    if (C.size() == 2) {
      if (C[0].second != C[1].second) {
        if (C[0].second) { // A || !B
          FeatureBuilder[C[1].first].addImplication(
              FeatureBuilder[C[0].first].get());
        } else { // !A || B
          FeatureBuilder[C[0].first].addImplication(
              FeatureBuilder[C[1].first].get());
        }
      } else if (!(C[0].second || C[1].second)) { // !A || !B
        FeatureBuilder[C[0].first].addExclude(FeatureBuilder[C[1].first].get());
        FeatureBuilder[C[1].first].addExclude(FeatureBuilder[C[0].first].get());
      } else if (C[0].second && C[1].second) { // A || B
        FeatureBuilder[C[0].first].addAlternative(
            FeatureBuilder[C[1].first].get());
        FeatureBuilder[C[1].first].addAlternative(
            FeatureBuilder[C[0].first].get());
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
              FeatureBuilder[P.first].addAlternative(
                  FeatureBuilder[PP.first].get());
            }
          }
        }
      }
    }
  }
}

bool FeatureModel::Builder::buildTree(const string &F) {
  if (FeatureBuilder[F].get()->getIndex() > 0) {
    llvm::errs() << "error: Cyclic feature model in \'" << F << "\'.\n";
    return false;
  }
  FeatureBuilder[F].setIndex(Index++);
  Features.push_back(FeatureBuilder[F].get());
  std::sort(Children[F].begin(), Children[F].end());
  for (const auto &C : Children[F]) {
    if (!buildTree(C)) {
      return false;
    }
    FeatureBuilder[F].addChild(FeatureBuilder[C].get());
  }
  return true;
}

void FeatureModel::Builder::buildFeatures() {
  for (const auto &F : FeatureBuilder.keys()) {
    FeatureMap.try_emplace(F, FeatureBuilder[F].build());
  }
}

void FeatureModel::Builder::setRoot(std::string R) {
  assert(Root.empty() && "Root already set.");
  Root = R;
  if (FeatureBuilder.find(Root) == FeatureBuilder.end()) {
    FeatureBuilder.try_emplace(Root, Root, false);
  }
  for (const auto &F : FeatureBuilder.keys()) {
    if (F != Root && Parents.find(F) == Parents.end()) {
      Children[Root].push_back(F);
      Parents[F] = Root;
    }
  }
}

std::unique_ptr<FeatureModel> FeatureModel::Builder::build() {
  if (Root.empty()) {
    setRoot();
  }
  Index = 1;
  assert(!Root.empty() && "Root not set.");
  if (!buildTree(Root)) {
    return nullptr;
  }
  buildConstraints();
  buildFeatures();
  return std::make_unique<FeatureModel>(VmName, RootPath, std::move(FeatureMap),
                                        Constraints, FeatureMap[Root].get(),
                                        Features);
}
} // namespace vara::feature
