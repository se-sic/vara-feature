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
        Features(FeatureModelModifyTransaction::openTransaction(*FM)),
        Transactions(FeatureModelModifyTransaction::openTransaction(*FM)),
        PostTransactions(FeatureModelModifyTransaction::openTransaction(*FM)),
        Special(FeatureModelModifyTransaction::openTransaction(*FM)) {}

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
    Features.addFeature(std::make_unique<FeatureTy>(
        FeatureName, std::forward<Args>(FurtherArgs)...));
    return this;
  }

  FeatureModelBuilder *addEdge(const std::string &ParentName,
                               const std::string &FeatureName) {
    Transactions.addChild(ParentName, FeatureName);
    return this;
  }

  FeatureModelBuilder *
  emplaceRelationship(Relationship::RelationshipKind RK,
                      const std::vector<std::string> &FeatureNames,
                      const std::string &ParentName) {
    PostTransactions.addRelationship(
        RK, ParentName,
        std::set<std::string>(FeatureNames.begin(), FeatureNames.end()));
    return this;
  }

  FeatureModelBuilder *
  addConstraint(std::unique_ptr<FeatureModel::ConstraintTy> C) {
    PostTransactions.addConstraint(std::move(C));
    return this;
  }

  FeatureModelBuilder *setVmName(std::string Name) {
    Transactions.setName(std::move(Name));
    return this;
  }

  FeatureModelBuilder *setPath(const fs::path &Path) {
    Transactions.setPath(Path);
    return this;
  }

  FeatureModelBuilder *setCommit(std::string Commit) {
    Transactions.setCommit(Commit);
    return this;
  }

  FeatureModelBuilder *makeRoot(const std::string &Name) {
    Transactions.setRoot(std::make_unique<RootFeature>(Name));
    return this;
  }

  /// Build \a FeatureModel.
  ///
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();

private:
  std::unique_ptr<FeatureModel> FM;
  FeatureModelTransaction<detail::ModifyTransactionMode> Features;
  FeatureModelTransaction<detail::ModifyTransactionMode> Transactions;
  FeatureModelTransaction<detail::ModifyTransactionMode> PostTransactions;
  FeatureModelTransaction<detail::ModifyTransactionMode> Special;

  /// This method is solely relevant for parsing XML, as alternatives are
  /// represented als mutual excluded but non-optional features (which requires
  /// additional processing).
  bool detectXMLAlternatives();
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELBUILDER_H
