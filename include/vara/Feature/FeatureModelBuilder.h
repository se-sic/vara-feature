#ifndef VARA_FEATURE_FEATUREMODELBUILDER_H
#define VARA_FEATURE_FEATUREMODELBUILDER_H

#include <utility>

#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

/// \brief Builder for \a FeatureModel which can be used while parsing.
class FeatureModelBuilder {
public:
  FeatureModelBuilder()
      : FM(std::make_unique<FeatureModel>()),
        FeatureBuilder(FeatureModelModifyTransaction::openTransaction(*FM)),
        ModelBuilder(FeatureModelModifyTransaction::openTransaction(*FM)),
        RelationBuilder(FeatureModelModifyTransaction::openTransaction(*FM)) {}

  FeatureModelBuilder(const FeatureModelBuilder &) = delete;
  FeatureModelBuilder &operator=(const FeatureModelBuilder &) = delete;
  FeatureModelBuilder(FeatureModelBuilder &&) noexcept = delete;
  FeatureModelBuilder &operator=(FeatureModelBuilder &&) noexcept = delete;

  ~FeatureModelBuilder() {
    FeatureBuilder.abort();
    ModelBuilder.abort();
    RelationBuilder.abort();
  }

  /// Try to create a new \a Feature.
  ///
  /// \param[in] FeatureName name of the \a Feature
  /// \param[in] FurtherArgs further arguments that should be passed to the
  ///                        \a Feature constructor
  ///
  /// \returns FeatureModelBuilder
  template <
      typename FeatureTy, typename... Args,
      std::enable_if_t<std::is_base_of_v<Feature, FeatureTy>, bool> = true>
  FeatureModelBuilder *makeFeature(std::string FeatureName,
                                   Args &&...FurtherArgs) {
    FeatureBuilder.addFeature(std::make_unique<FeatureTy>(
        FeatureName, std::forward<Args>(FurtherArgs)...));
    return this;
  }

  FeatureModelBuilder *addEdge(const std::string &ParentName,
                               const std::string &FeatureName) {
    Parents[FeatureName] = ParentName;
    ModelBuilder.addChild(ParentName, FeatureName);
    return this;
  }

  std::optional<std::string> getParentName(const std::string &FeatureName) {
    if (const auto *P = Parents.find(FeatureName); P != Parents.end()) {
      return P->getValue();
    }
    return std::nullopt;
  }

  FeatureModelBuilder *emplaceRelationship(Relationship::RelationshipKind RK,
                                           const std::string &ParentName) {
    RelationBuilder.addRelationship(RK, ParentName);
    return this;
  }

  FeatureModelBuilder *
  addConstraint(std::unique_ptr<FeatureModel::ConstraintTy> C) {
    RelationBuilder.addConstraint(std::move(C));
    return this;
  }

  FeatureModelBuilder *setVmName(std::string Name) {
    ModelBuilder.setName(std::move(Name));
    return this;
  }

  FeatureModelBuilder *setPath(const fs::path &Path) {
    ModelBuilder.setPath(Path);
    return this;
  }

  FeatureModelBuilder *setCommit(std::string Commit) {
    ModelBuilder.setCommit(std::move(Commit));
    return this;
  }

  FeatureModelBuilder *makeRoot(const std::string &Name) {
    ModelBuilder.setRoot(std::make_unique<RootFeature>(Name));
    return this;
  }

  /// Build \a FeatureModel.
  ///
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();

private:
  std::unique_ptr<FeatureModel> FM;
  llvm::StringMap<std::string> Parents;
  // Modifications to initialize features as children of root.
  FeatureModelTransaction<detail::ModifyTransactionMode> FeatureBuilder;
  // Modifications to build tree structure and set FM meta information.
  FeatureModelTransaction<detail::ModifyTransactionMode> ModelBuilder;
  // Modifications to add relationships and constraints.
  FeatureModelTransaction<detail::ModifyTransactionMode> RelationBuilder;
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELBUILDER_H
