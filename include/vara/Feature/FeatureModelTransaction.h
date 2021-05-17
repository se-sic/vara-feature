#ifndef VARA_FEATURE_FEATUREMODELTRANSACTION_H
#define VARA_FEATURE_FEATUREMODELTRANSACTION_H

#include "vara/Feature/FeatureModel.h"
#include "vara/Utils/VariantUtil.h"

#include "llvm/ADT/StringRef.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace vara::feature {

namespace detail {
class CopyTransactionMode;
class ModifyTransactionMode;
class FeatureModelCopyTransactionBase;
class FeatureModelModifyTransactionBase;

using ConsistencyCheck =
    FeatureModelConsistencyChecker<ExactlyOneRootNode,
                                   EveryFeatureRequiresParent,
                                   CheckFeatureParentChildRelationShip>;

using FeatureVariantTy = std::variant<std::string, Feature *>;
using FeatureTreeNodeVariantTy = std::variant<std::string, FeatureTreeNode *>;
} // namespace detail

template <typename CopyMode>
class FeatureModelTransaction
    : private std::conditional_t<
          std::is_same_v<CopyMode, detail::CopyTransactionMode>,
          detail::FeatureModelCopyTransactionBase,
          detail::FeatureModelModifyTransactionBase> {
  static inline constexpr bool IsCopyMode =
      std::is_same_v<CopyMode, detail::CopyTransactionMode>;
  using TransactionBaseTy =
      std::conditional_t<IsCopyMode, detail::FeatureModelCopyTransactionBase,
                         detail::FeatureModelModifyTransactionBase>;

public:
  /// \brief Opens a new Transaction for the given FeatureModel. The Transaction
  /// enables the user to modify the underlying FeatureModel via a specific
  /// API, preserving the correctness of the FeatureModel.
  static FeatureModelTransaction openTransaction(FeatureModel &FM) {
    assert(detail::ConsistencyCheck::isFeatureModelValid(FM) &&
           "Passed FeatureModel was in an invalid state.");
    return FeatureModelTransaction(FM);
  }

  // Transaction can only be moved. Copying a currently open Transaction has no
  // well defined meaning.
  FeatureModelTransaction(const FeatureModelTransaction &) = delete;
  FeatureModelTransaction &operator=(const FeatureModelTransaction &) = delete;
  FeatureModelTransaction(FeatureModelTransaction &&) noexcept = default;
  FeatureModelTransaction &
  operator=(FeatureModelTransaction &&) noexcept = default;
  ~FeatureModelTransaction() {
    if constexpr (IsCopyMode) {
      assert(!this->isUncommitted() &&
             "Transaction in CopyMode should be committed before destruction.");
    } else {
      if (this->isUncommitted()) {
        // In modification mode we should ensure that changes are committed
        //  before destruction
        llvm::errs()
            << "warning: Uncommitted modifications before destruction.\n";
        // TODO(se-passau/VaRA#744): Committing now may break with prior
        //  failed commits. We need a better way of tracking if a previous
        //  commit failed, instead of checking locally if FM is nullptr.
        commit();
      }
    }
  }

  /// \brief Commit and finalize the FeatureModelTransaction.
  /// In CopyMode a new FeatureModel is returned.
  /// In ModifyMode the specified changes are applied to the underlying
  /// FeatureModel.
  ///
  /// \returns a FeatureModel or nothing
  decltype(auto) commit() { return this->commitImpl(); }

  /// Abort the current Transaction, throwing away all changes.
  void abort() { this->abortImpl(); }

  /// \brief Add a Feature to the FeatureModel
  ///
  /// \returns a pointer to the inserted Feature in CopyMode, otherwise,
  ///          nothing.
  decltype(auto) addFeature(std::unique_ptr<Feature> NewFeature,
                            Feature *Parent = nullptr) {
    if constexpr (IsCopyMode) {
      return this->addFeatureImpl(std::move(NewFeature), Parent);
    } else {
      this->addFeatureImpl(std::move(NewFeature), Parent);
    }
  }

  void removeFeature(detail::FeatureVariantTy &F, bool Recursive = false) {

    this->removeFeatureImpl(F, Recursive);
  }

  decltype(auto) addRelationship(Relationship::RelationshipKind Kind,
                                 const detail::FeatureVariantTy &Parent) {
    if constexpr (IsCopyMode) {
      return this->addRelationshipImpl(Kind, Parent);
    } else {
      this->addRelationshipImpl(Kind, Parent);
    }
  }

  void removeRelationship(detail::FeatureVariantTy &F) {
    this->removeRelationshipImpl(F);
  }

  decltype(auto) addLocation(const detail::FeatureVariantTy &F,
                             FeatureSourceRange FSR) {
    if constexpr (IsCopyMode) {
      return this->addLocationImpl(F, FSR);
    } else {
      this->addLocationImpl(F, FSR);
    }
  }

  void removeLocation(detail::FeatureVariantTy &F, FeatureSourceRange FSR) {
    this->removeLocationImpl(F, FSR);
  }

  decltype(auto) addConstraint(std::unique_ptr<Constraint> Constraint) {
    if constexpr (IsCopyMode) {
      return this->addConstraintImpl(std::move(Constraint));
    } else {
      this->addConstraintImpl(std::move(Constraint));
    }
  }

  void setName(std::string Name) { return this->setNameImpl(std::move(Name)); }

  void setCommit(std::string Commit) {
    return this->setCommitImpl(std::move(Commit));
  }

  void setPath(fs::path Path) { return this->setPathImpl(std::move(Path)); }

  decltype(auto) setRoot(std::unique_ptr<RootFeature> Root) {
    if constexpr (IsCopyMode) {
      return this->setRootImpl(std::move(Root));
    } else {
      this->setRootImpl(std::move(Root));
    }
  }

  decltype(auto) addChild(const detail::FeatureTreeNodeVariantTy &Parent,
                          const detail::FeatureTreeNodeVariantTy &Child) {
    if constexpr (IsCopyMode) {
      return this->addChildImpl(Parent, Child);
    } else {
      this->addChildImpl(Parent, Child);
    }
  }

private:
  FeatureModelTransaction(FeatureModel &FM) : TransactionBaseTy(FM) {}
};

using FeatureModelCopyTransaction =
    FeatureModelTransaction<detail::CopyTransactionMode>;
using FeatureModelModifyTransaction =
    FeatureModelTransaction<detail::ModifyTransactionMode>;

//===----------------------------------------------------------------------===//
//                    Transaction Implementation Details
//===----------------------------------------------------------------------===//

namespace detail {

class CopyTransactionMode {};
class ModifyTransactionMode {};

class FeatureModelModification {
  friend class FeatureModelCopyTransactionBase;
  friend class FeatureModelModifyTransactionBase;

public:
  virtual ~FeatureModelModification() = default;

  /// \brief Execute the modification on the given FeatureModel.
  virtual void exec(FeatureModel &FM) = 0;

protected:
  /// \brief Set the parent of a \a Feature.
  static void setParent(FeatureTreeNode &F, FeatureTreeNode &Parent) {
    F.setParent(&Parent);
  }

  /// \brief Remove the parent of a \a Feature.
  static void removeParent(FeatureTreeNode &F) { F.setParent(nullptr); }

  /// \brief Add a \a Feature Child to F.
  static void addEdge(FeatureTreeNode &F, FeatureTreeNode &Child) {
    F.addEdge(&Child);
  }

  /// \brief Remove \a Feature Child from F.
  static void removeEdge(FeatureTreeNode &F, FeatureTreeNode &Child) {
    F.removeEdge(&Child);
  }

  static void addConstraint(Feature &F, PrimaryFeatureConstraint &Constraint) {
    F.addConstraint(&Constraint);
  }

  static void addLocation(Feature &F, FeatureSourceRange FSR) {
    F.addLocation(std::move(FSR));
  }

  static void removeLocation(Feature &F, const FeatureSourceRange &FSR) {
    F.removeLocation(FSR);
  }

  static void setFeature(PrimaryFeatureConstraint &Constraint, Feature &F) {
    Constraint.setFeature(&F);
  }

  /// \brief Adds a new \a Feature to the FeatureModel.
  ///
  /// \param FM model to add to
  /// \param NewFeature the Feature to add
  ///
  /// \returns A pointer to the inserted Feature.
  static Feature *addFeature(FeatureModel &FM,
                             std::unique_ptr<Feature> NewFeature) {
    return FM.addFeature(std::move(NewFeature));
  }

  static Relationship *
  addRelationship(FeatureModel &FM,
                  std::unique_ptr<Relationship> NewRelationship) {
    return FM.addRelationship(std::move(NewRelationship));
  }

  static void removeRelationship(FeatureModel &FM, Relationship *R) {
    FM.removeRelationship(R);
  }

  static Constraint *addConstraint(FeatureModel &FM,
                                   std::unique_ptr<Constraint> Constraint) {
    return FM.addConstraint(std::move(Constraint));
  }

  static void setName(FeatureModel &FM, std::string NewName) {
    FM.setName(std::move(NewName));
  }

  static void setCommit(FeatureModel &FM, std::string NewCommit) {
    FM.setCommit(std::move(NewCommit));
  }

  static void setPath(FeatureModel &FM, fs::path NewPath) {
    FM.setPath(std::move(NewPath));
  }

  static RootFeature *setRoot(FeatureModel &FM, RootFeature &NewRoot) {
    return FM.setRoot(NewRoot);
  }

  /// \brief Remove \a Feature from a \a FeatureModel.
  ///
  /// \param FM model to remove from
  /// \param F the Feature to delete
  static void removeFeature(FeatureModel &FM, Feature &F) {
    FM.removeFeature(F);
  }

  template <typename ModTy, typename... ArgTys>
  static ModTy make_modification(ArgTys &&...Args) {
    return ModTy(std::forward<ArgTys>(Args)...);
  }

  template <typename ModTy, typename... ArgTys>
  static std::unique_ptr<ModTy> make_unique_modification(ArgTys &&...Args) {
    return std::unique_ptr<ModTy>(new ModTy(std::forward<ArgTys>(Args)...));
  }
};

//===----------------------------------------------------------------------===//
//                          AddFeatureToModel
//===----------------------------------------------------------------------===//

class AddFeatureToModel : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  Feature *operator()(FeatureModel &FM) {
    auto *InsertedFeature = addFeature(FM, std::move(NewFeature));
    if (!InsertedFeature) {
      return nullptr;
    }
    if (Parent) {
      setParent(*InsertedFeature, *Parent);
      addEdge(*Parent, *InsertedFeature);
    } else if (FM.getRoot()) {
      setParent(*InsertedFeature, *FM.getRoot());
      addEdge(*FM.getRoot(), *InsertedFeature);
    }
    return InsertedFeature;
  }

private:
  AddFeatureToModel(std::unique_ptr<Feature> NewFeature,
                    Feature *Parent = nullptr)
      : NewFeature(std::move(NewFeature)), Parent(Parent) {}

  std::unique_ptr<Feature> NewFeature;
  Feature *Parent;
};

//===----------------------------------------------------------------------===//
//                          RemoveFeatureFromModel
//===----------------------------------------------------------------------===//

class RemoveFeatureFromModel : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    auto *F = std::visit(
        Overloaded{
            [&FM](const std::string &Name) { return FM.getFeature(Name); },
            [](Feature *Ptr) { return Ptr; },
        },
        GroupRoot);
    if (!F) {
      // TODO (se-passau/VaRA#744): error, does not exist
      return;
    }

    if (Recursive) {
      for (auto *C : F->getChildren<Feature>()) {
        // TODO (se-passau/VaRA#744): error, if call returns error (should not
        // be possible)
        RemoveFeatureFromModel(C, Recursive)(FM);
      }
      for (auto *R : F->getChildren<Relationship>()) {
        removeEdge(*F, *R);
        removeRelationship(FM, R);
      }
    } else {
      if (!F->getChildren<Feature>().empty()) {
        // TODO (se-passau/VaRA#744): error, non recursive 0remove on non leaf
        // feature
        return;
      }
      if (!F->getChildren<Relationship>().empty()) {
        removeRelationship(FM, *F->getChildren<Relationship>().begin());
      }
    }

    removeEdge(*F->getParent(), *F);
    removeFeature(FM, *F);
  }

private:
  RemoveFeatureFromModel(FeatureVariantTy GroupRoot, bool Recursive = false)
      : GroupRoot(std::move(GroupRoot)), Recursive(Recursive) {}

  FeatureVariantTy GroupRoot;
  bool Recursive;
};

//===----------------------------------------------------------------------===//
//                              AddRelationshipToModel
//===----------------------------------------------------------------------===//

class AddRelationshipToModel : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  Relationship *operator()(FeatureModel &FM) {
    auto *InsertedRelationship =
        addRelationship(FM, std::make_unique<Relationship>(Kind));
    if (!InsertedRelationship) {
      return nullptr;
    }

    auto *P = std::visit(
        Overloaded{
            [&FM](const std::string &Name) { return FM.getFeature(Name); },
            [](Feature *Ptr) { return Ptr; },
        },
        Parent);

    setParent(*InsertedRelationship, *P);
    addEdge(*P, *InsertedRelationship);

    for (auto *C : P->getChildren<Feature>()) {
      assert(P->hasEdgeTo(*C) && C->hasEdgeFrom(*P));
      removeEdge(*P, *C);
      addEdge(*InsertedRelationship, *C);
      setParent(*C, *InsertedRelationship);
    }

    return InsertedRelationship;
  }

private:
  AddRelationshipToModel(Relationship::RelationshipKind Kind,
                         FeatureVariantTy Parent)
      : Kind(Kind), Parent(std::move(Parent)) {}

  Relationship::RelationshipKind Kind;
  FeatureVariantTy Parent;
};

//===----------------------------------------------------------------------===//
//                              RemoveRelationshipFromModel
//===----------------------------------------------------------------------===//

class RemoveRelationshipFromModel : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    auto *P = std::visit(
        Overloaded{
            [&FM](const std::string &Name) { return FM.getFeature(Name); },
            [](Feature *Ptr) { return Ptr; },
        },
        Parent);
    if (!P) {
      // TODO(se-passau/VaRA#744) handle this
      return;
    }

    auto Relationships = P->getChildren<Relationship>();
    Relationship *R = *Relationships.begin();

    removeEdge(*P, *R);

    for (auto *C : R->getChildren<Feature>()) {
      assert(R->hasEdgeTo(*C) && C->hasEdgeFrom(*R));
      removeEdge(*R, *C);
      addEdge(*P, *C);
      setParent(*C, *P);
    }
    removeRelationship(FM, R);
  }

private:
  RemoveRelationshipFromModel(FeatureVariantTy Parent)
      : Parent(std::move(Parent)) {}

  FeatureVariantTy Parent;
};

//===----------------------------------------------------------------------===//
//                          AddLocationToFeature
//===----------------------------------------------------------------------===//

class AddLocationToFeature : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    addLocation(*std::visit(Overloaded{
                                [&FM](const std::string &Name) {
                                  return FM.getFeature(Name);
                                },
                                [](Feature *Ptr) { return Ptr; },
                            },
                            F),
                FSR);
  }

private:
  AddLocationToFeature(FeatureVariantTy F, FeatureSourceRange FSR)
      : F(std::move(F)), FSR(std::move(FSR)) {}

  FeatureVariantTy F;
  FeatureSourceRange FSR;
};

//===----------------------------------------------------------------------===//
//                          RemoveLocationFromFeature
//===----------------------------------------------------------------------===//

class RemoveLocationFromFeature : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    removeLocation(*std::visit(Overloaded{
                                   [&FM](const std::string &Name) {
                                     return FM.getFeature(Name);
                                   },
                                   [](Feature *Ptr) { return Ptr; },
                               },
                               F),
                   FSR);
  }

private:
  RemoveLocationFromFeature(FeatureVariantTy F, FeatureSourceRange FSR)
      : F(std::move(F)), FSR(std::move(FSR)) {}

  FeatureVariantTy F;
  FeatureSourceRange FSR;
};

//===----------------------------------------------------------------------===//
//                       AddConstraintToModel
//===----------------------------------------------------------------------===//

class AddConstraintToModel : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  Constraint *operator()(FeatureModel &FM) {
    auto *InsertedConstraint = addConstraint(FM, std::move(NewConstraint));
    if (!InsertedConstraint) {
      return nullptr;
    }
    auto V = AddConstraintToModelVisitor(&FM);
    InsertedConstraint->accept(V);
    return InsertedConstraint;
  }

private:
  class AddConstraintToModelVisitor : public ConstraintVisitor {

  public:
    AddConstraintToModelVisitor(FeatureModel *FM) : FM(FM) {}

    void visit(PrimaryFeatureConstraint *C) override {
      auto *F = FM->getFeature(C->getFeature()->getName());
      AddConstraintToModel::setFeature(*C, *F);
      AddConstraintToModel::addConstraint(*F, *C);
    };

  private:
    FeatureModel *FM;
  };

  AddConstraintToModel(std::unique_ptr<Constraint> NewConstraint)
      : NewConstraint(std::move(NewConstraint)) {}

  std::unique_ptr<Constraint> NewConstraint;
};

//===----------------------------------------------------------------------===//
//                              SetName
//===----------------------------------------------------------------------===//

class SetName : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) { setName(FM, std::move(Name)); }

private:
  SetName(std::string Name) : Name(std::move(Name)) {}

  std::string Name;
};

//===----------------------------------------------------------------------===//
//                              SetCommit
//===----------------------------------------------------------------------===//

class SetCommit : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) { setCommit(FM, std::move(Commit)); }

private:
  SetCommit(std::string Commit) : Commit(std::move(Commit)) {}

  std::string Commit;
};

//===----------------------------------------------------------------------===//
//                              SetPath
//===----------------------------------------------------------------------===//

class SetPath : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) { setPath(FM, std::move(Path)); }

private:
  SetPath(fs::path Path) : Path(std::move(Path)) {}

  fs::path Path;
};

//===----------------------------------------------------------------------===//
//                              SetRoot
//===----------------------------------------------------------------------===//

class SetRoot : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  RootFeature *operator()(FeatureModel &FM) {
    if (auto *NewRoot = llvm::dyn_cast_or_null<RootFeature>(
            addFeature(FM, std::move(Root)));
        NewRoot) {
      if (FM.getRoot()) {
        for (auto *C : FM.getRoot()->children()) {
          setParent(*C, *NewRoot);
          removeEdge(*FM.getRoot(), *C);
          addEdge(*NewRoot, *C);
        }
        removeFeature(FM, *FM.getRoot());
      }
      setRoot(FM, *NewRoot);
    }
    return FM.getRoot();
  }

private:
  SetRoot(std::unique_ptr<RootFeature> Root) : Root(std::move(Root)) {}

  std::unique_ptr<RootFeature> Root;
};

//===----------------------------------------------------------------------===//
//                              AddChild
//===----------------------------------------------------------------------===//

class AddChild : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    auto *C = std::visit(Overloaded{
                             [&FM](const std::string &Name) {
                               return llvm::dyn_cast_or_null<FeatureTreeNode>(
                                   FM.getFeature(Name));
                             },
                             [](FeatureTreeNode *Ptr) { return Ptr; },
                         },
                         Child);
    auto *P = std::visit(Overloaded{
                             [&FM](const std::string &Name) {
                               return llvm::dyn_cast_or_null<FeatureTreeNode>(
                                   FM.getFeature(Name));
                             },
                             [](FeatureTreeNode *Ptr) { return Ptr; },
                         },
                         Parent);
    assert(C && P);

    removeEdge(*C->getParent(), *C);
    addEdge(*P, *C);
    setParent(*C, *P);
  }

private:
  AddChild(FeatureTreeNodeVariantTy Parent, FeatureTreeNodeVariantTy Child)
      : Child(std::move(Child)), Parent(std::move(Parent)) {}

  FeatureTreeNodeVariantTy Child;
  FeatureTreeNodeVariantTy Parent;
};

class FeatureModelCopyTransactionBase {
protected:
  FeatureModelCopyTransactionBase(FeatureModel &FM) : FM(FM.clone()) {}

  [[nodiscard]] inline std::unique_ptr<FeatureModel> commitImpl() {
    if (FM && ConsistencyCheck::isFeatureModelValid(*FM)) {
      return std::move(FM);
    }
    return nullptr;
  };

  void abortImpl() { FM.reset(); }

  [[nodiscard]] inline bool isUncommitted() const { return FM != nullptr; }

  //===--------------------------------------------------------------------===//
  // Modifications

  Feature *addFeatureImpl(std::unique_ptr<Feature> NewFeature,
                          Feature *Parent) {
    if (!FM) {
      return nullptr;
    }

    if (Parent) {
      // To correctly add a parent, we need to translate it to a Feature in
      // our copied FeatureModel
      return FeatureModelModification::make_modification<AddFeatureToModel>(
          std::move(NewFeature), TranslateFeature(*Parent))(*FM);
    }

    return FeatureModelModification::make_modification<AddFeatureToModel>(
        std::move(NewFeature))(*FM);
  }

  void removeFeatureImpl(FeatureVariantTy &F, bool Recursive = false) {
    assert(FM && "");

    FeatureModelModification::make_modification<RemoveFeatureFromModel>(
        TranslateFeature(*std::visit(Overloaded{
                                         [this](const std::string &Name) {
                                           return FM->getFeature(Name);
                                         },
                                         [](Feature *Ptr) { return Ptr; },
                                     },
                                     F)),
        Recursive)(*FM);
  }

  Relationship *addRelationshipImpl(Relationship::RelationshipKind Kind,
                                    const FeatureVariantTy &Parent) {
    if (!FM) {
      return nullptr;
    }

    return FeatureModelModification::make_modification<AddRelationshipToModel>(
        Kind, TranslateFeature(*std::visit(Overloaded{
                                               [this](const std::string &Name) {
                                                 return FM->getFeature(Name);
                                               },
                                               [](Feature *Ptr) { return Ptr; },
                                           },
                                           Parent)))(*FM);
  }

  void removeRelationshipImpl(FeatureVariantTy &F) {
    assert(FM && "");

    return FeatureModelModification::make_modification<
        RemoveRelationshipFromModel>(TranslateFeature(*std::visit(
        Overloaded{
            [this](const std::string &Name) { return FM->getFeature(Name); },
            [](Feature *Ptr) { return Ptr; },
        },
        F)))(*FM);
  }

  void addLocationImpl(const FeatureVariantTy &F, FeatureSourceRange FSR) {
    assert(FM && "FeatureModel is null.");

    FeatureModelModification::make_modification<AddLocationToFeature>(
        TranslateFeature(*std::visit(Overloaded{
                                         [this](const std::string &Name) {
                                           return FM->getFeature(Name);
                                         },
                                         [](Feature *Ptr) { return Ptr; },
                                     },
                                     F)),
        std::move(FSR))(*FM);
  }

  void removeLocationImpl(const FeatureVariantTy &F, FeatureSourceRange &FSR) {
    assert(FM && "");

    FeatureModelModification::make_modification<RemoveLocationFromFeature>(
        TranslateFeature(*std::visit(Overloaded{
                                         [this](const std::string &Name) {
                                           return FM->getFeature(Name);
                                         },
                                         [](Feature *Ptr) { return Ptr; },
                                     },
                                     F)),
        FSR)(*FM);
  }

  Constraint *addConstraintImpl(std::unique_ptr<Constraint> NewConstraint) {
    if (!FM) {
      return nullptr;
    }

    return FeatureModelModification::make_modification<AddConstraintToModel>(
        std::move(NewConstraint))(*FM);
  }

  void setNameImpl(std::string Name) {
    assert(FM && "FeatureModel is null.");

    FeatureModelModification::make_modification<SetName>(std::move(Name))(*FM);
  }

  void setCommitImpl(std::string Commit) {
    assert(FM && "FeatureModel is null.");

    FeatureModelModification::make_modification<SetCommit>(std::move(Commit))(
        *FM);
  }

  void setPathImpl(fs::path Path) {
    assert(FM && "FeatureModel is null.");

    FeatureModelModification::make_modification<SetPath>(std::move(Path))(*FM);
  }

  RootFeature *setRootImpl(std::unique_ptr<RootFeature> Root) {
    if (!FM) {
      return nullptr;
    }

    return FeatureModelModification::make_modification<SetRoot>(
        std::move(Root))(*FM);
  }

  static void addChildImpl(const FeatureTreeNodeVariantTy &Parent,
                           const FeatureTreeNodeVariantTy &Child) {
    FeatureModelModification::make_modification<AddChild>(Parent, Child);
  }

private:
  [[nodiscard]] Feature *TranslateFeature(Feature &F) {
    return FM->getFeature(F.getName());
  }

  std::unique_ptr<FeatureModel> FM;
};

class FeatureModelModifyTransactionBase {
protected:
  FeatureModelModifyTransactionBase(FeatureModel &FM) : FM(&FM) {}

  bool commitImpl() {
    assert(FM && "Cannot commit Modifications without a FeatureModel present.");
    if (FM) {
      std::for_each(
          Modifications.begin(), Modifications.end(),
          [this](const std::unique_ptr<FeatureModelModification> &FMM) {
            FMM->exec(*FM);
          });
      if (ConsistencyCheck::isFeatureModelValid(*FM)) {
        FM = nullptr;
        return true;
      }
      // TODO (se-passau/VaRA#723): implement rollback
    }
    return false;
  };

  void abortImpl() { Modifications.clear(); };

  [[nodiscard]] inline bool isUncommitted() const { return FM != nullptr; }

  //===--------------------------------------------------------------------===//
  // Modifications

  void addFeatureImpl(std::unique_ptr<Feature> NewFeature, Feature *Parent) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<AddFeatureToModel>(
            std::move(NewFeature), Parent));
  }

  void removeFeatureImpl(FeatureVariantTy &F, bool Recursive = false) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            RemoveFeatureFromModel>(F, Recursive));
  }

  void addRelationshipImpl(Relationship::RelationshipKind Kind,
                           const FeatureVariantTy &Parent) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            AddRelationshipToModel>(Kind, Parent));
  }

  void removeRelationshipImpl(FeatureVariantTy &F) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            RemoveRelationshipFromModel>(F));
  }

  void addLocationImpl(const FeatureVariantTy &F, FeatureSourceRange FSR) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            AddLocationToFeature>(F, std::move(FSR)));
  }

  void removeLocationImpl(const FeatureVariantTy &F, FeatureSourceRange &FSR) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            RemoveLocationFromFeature>(F, FSR));
  }

  void addConstraintImpl(std::unique_ptr<Constraint> NewConstraint) {
    Modifications.push_back(FeatureModelModification::make_unique_modification<
                            AddConstraintToModel>(std::move(NewConstraint)));
  }

  void setNameImpl(std::string Name) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetName>(
            std::move(Name)));
  }

  void setCommitImpl(std::string Commit) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetCommit>(
            std::move(Commit)));
  }

  void setPathImpl(fs::path Path) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetPath>(
            std::move(Path)));
  }

  void setRootImpl(std::unique_ptr<RootFeature> Root) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetRoot>(
            std::move(Root)));
  }

  void addChildImpl(const FeatureTreeNodeVariantTy &Parent,
                    const FeatureTreeNodeVariantTy &Child) {
    assert(FM && "FeatureModel is null.");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<AddChild>(Parent,
                                                                     Child));
  }

private:
  FeatureModel *FM;
  std::vector<std::unique_ptr<FeatureModelModification>> Modifications;
};

} // namespace detail

//===----------------------------------------------------------------------===//
//                            Modification Helpers
//===----------------------------------------------------------------------===//

/// Adds a Feature to the FeatureModel
///
/// If a Parent is passed it needs to be already in the FeatureModel,
/// otherwise, root is assumed as the parent Feature.
///
/// \param FM
/// \param NewFeature
/// \param Parent of the new feature
void addFeature(FeatureModel &FM, std::unique_ptr<Feature> NewFeature,
                Feature *Parent = nullptr);

/// Adds multiple Features to the FeatureModel
///
/// The vector of Features contains tuples of a parent and a new Feature.
/// If a Parent is passed it needs to be already in the FeatureModel,
/// otherwise, root is assumed as the parent Feature.
///
/// \param FM
/// \param NewFeatures
void addFeatures(
    FeatureModel &FM,
    std::vector<std::pair<std::unique_ptr<Feature>, Feature *>> NewFeatures);

/// Removes a Feature from the FeatureModel
///
/// \param FM
/// \param FeatureToBeDeleted
void removeFeature(FeatureModel &FM,
                   detail::FeatureVariantTy FeatureToBeDeleted,
                   bool Recursive = false);

/// Adds a Relationship to a Feature
///
/// \param FM
/// \param GroupRoot of relationship group
/// \param Kind of relationship
void addRelationship(FeatureModel &FM, Feature *GroupRoot,
                     Relationship::RelationshipKind Kind);

/// Removes the Relationship from Group
///
/// \param FM
/// \param GroupRoot of Relationship
void removeRelationship(FeatureModel &FM, detail::FeatureVariantTy &GroupRoot);

/// Set commit of a FeatureModel.
///
/// \param FM
/// \param NewCommit
void setCommit(FeatureModel &FM, std::string NewCommit);

/// Merges a FeatureModel into another
///
/// Strict merging fails if both FeatureModels contain a Feature with equal
/// name, but different properties. Non-strict merging prefers properties of
/// Feature in \p FM1.
///
/// \param FM1
/// \param FM2
/// \param Strict mode
///
/// \return New merged FeatureModel or nullptr if merging failed
[[nodiscard]] std::unique_ptr<FeatureModel>
mergeFeatureModels(FeatureModel &FM1, FeatureModel &FM2, bool Strict = true);

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELTRANSACTION_H
