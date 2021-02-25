#ifndef VARA_FEATURE_FEATUREMODELBUILDER_H
#define VARA_FEATURE_FEATUREMODELBUILDER_H

#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                        FeatureModelBuilder
//===----------------------------------------------------------------------===//

/// \brief Builder for \a FeatureModel which can be used while parsing.
class FeatureModelBuilder : private FeatureModel {
public:
  FeatureModelBuilder() { init(); }

  void init() {
    Name = "";
    Path = "";
    Commit = "";
    Features.clear();
    Constraints.clear();
    Parents.clear();
    Children.clear();
    auto FMT = FeatureModelModifyTransaction::openTransaction(*this);
    FMT.setRoot(std::make_unique<RootFeature>(RootName));
    FMT.commit();
  }

  /// Try to create a new \a Feature.
  ///
  /// \param[in] FeatureName name of the \a Feature
  /// \param[in] FurtherArgs further arguments that should be passed to the
  ///                        \a Feature constructor
  ///
  /// \returns ptr to inserted \a Feature
  template <
      typename FeatureTy, typename... Args,
      std::enable_if_t<std::is_base_of_v<Feature, FeatureTy>, bool> = true>
  FeatureTy *makeFeature(std::string FeatureName, Args &&...FurtherArgs) {
    auto FMT = FeatureModelModifyTransaction::openTransaction(*this);
    FMT.addFeature(std::make_unique<FeatureTy>(
        FeatureName, std::forward<Args>(FurtherArgs)...));
    FMT.commit();
    return llvm::dyn_cast<FeatureTy>(getFeature(FeatureName));
  }

  FeatureModelBuilder *addEdge(const std::string &ParentName,
                               const std::string &FeatureName) {
    Children[ParentName].insert(FeatureName);
    Parents[FeatureName] = ParentName;
    return this;
  }

  FeatureModelBuilder *
  emplaceRelationship(Relationship::RelationshipKind RK,
                      const std::vector<std::string> &FeatureNames,
                      const std::string &ParentName) {
    RelationshipEdges[ParentName].emplace_back(RK, FeatureNames);
    return this;
  }

  FeatureModelBuilder *
  addConstraint(std::unique_ptr<FeatureModel::ConstraintTy> C) {
    Constraints.push_back(std::move(C));
    return this;
  }

  FeatureModelBuilder *setVmName(std::string Name) {
    this->Name = std::move(Name);
    return this;
  }

  FeatureModelBuilder *setPath(fs::path Path) {
    this->Path = std::move(Path);
    return this;
  }

  FeatureModelBuilder *setCommit(std::string Commit) {
    this->Commit = std::move(Commit);
    return this;
  }

  FeatureModelBuilder *setRoot(std::string Name) {
    this->RootName = std::move(Name);
    auto FMT = FeatureModelModifyTransaction::openTransaction(*this);
    FMT.setRoot(std::make_unique<RootFeature>(RootName));
    // TODO(s9latimm) remove old root
    FMT.commit();
    return this;
  }

  /// Build \a FeatureModel.
  ///
  /// \return instance of \a FeatureModel
  std::unique_ptr<FeatureModel> buildFeatureModel();

private:
  class BuilderVisitor : public ConstraintVisitor {

  public:
    BuilderVisitor(FeatureModelBuilder *Builder) : Builder(Builder) {}

    void visit(PrimaryFeatureConstraint *C) override {
      auto *F = Builder->getFeature(C->getFeature()->getName());
      C->setFeature(F);
      F->addConstraint(C);
    };

  private:
    FeatureModelBuilder *Builder;
  };

  using EdgeMapType = typename llvm::StringMap<llvm::SmallSet<std::string, 3>>;
  using RelationshipEdgeType = typename llvm::StringMap<std::vector<
      std::pair<Relationship::RelationshipKind, std::vector<std::string>>>>;

  FeatureModel::ConstraintContainerTy Constraints;
  FeatureModel::RelationshipContainerTy Relationships;
  llvm::StringMap<std::string> Parents;
  EdgeMapType Children;
  RelationshipEdgeType RelationshipEdges;
  std::string RootName{"root"};

  bool buildConstraints();

  /// This method is solely relevant for parsing XML, as alternatives are
  /// represented als mutual excluded but non-optional features (which requires
  /// additional processing).
  void detectXMLAlternatives();

  bool buildRoot();

  bool buildTree(const std::string &FeatureName,
                 std::set<std::string> &Visited);
};

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELBUILDER_H
