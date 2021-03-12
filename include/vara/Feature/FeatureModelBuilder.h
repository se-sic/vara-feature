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
        PostTransactions(FeatureModelModifyTransaction::openTransaction(*FM)) {}

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
    Constraints.push_back(std::move(C));
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
    //    this->Commit = std::move(Commit);
    return this;
  }

  FeatureModelBuilder *setRoot(const std::string &Name) {
    Transactions.setRoot(std::make_unique<RootFeature>(Name));
    return this;
  }

  /// Build \a FeatureModel.
  ///
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();

private:
  class BuilderVisitor : public ConstraintVisitor {

  public:
    BuilderVisitor(FeatureModelBuilder &Builder) : Builder(Builder) {}

    void visit(PrimaryFeatureConstraint *C) override {
      auto *F = Builder.FM->getFeature(C->getFeature()->getName());
      C->setFeature(F);
      F->addConstraint(C);
    };

  private:
    FeatureModelBuilder &Builder;
  };

  std::unique_ptr<FeatureModel> FM;
  FeatureModelTransaction<detail::ModifyTransactionMode> Features;
  FeatureModelTransaction<detail::ModifyTransactionMode> Transactions;
  FeatureModelTransaction<detail::ModifyTransactionMode> PostTransactions;

  using RelationshipEdgeType = typename llvm::StringMap<std::vector<
      std::pair<Relationship::RelationshipKind, std::vector<std::string>>>>;

  FeatureModel::ConstraintContainerTy Constraints;
  FeatureModel::RelationshipContainerTy Relationships;
  RelationshipEdgeType RelationshipEdges;

  bool buildConstraints();

  /// This method is solely relevant for parsing XML, as alternatives are
  /// represented als mutual excluded but non-optional features (which requires
  /// additional processing).
  void detectXMLAlternatives();

  bool buildRoot();

  bool buildTree(Feature &F, std::set<std::string> &Visited);
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELBUILDER_H
