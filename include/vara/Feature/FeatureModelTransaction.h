#ifndef VARA_FEATURE_FEATUREMODELTRANSACTION_H
#define VARA_FEATURE_FEATUREMODELTRANSACTION_H

#include "vara/Feature/FeatureModel.h"

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
    FeatureModelConsistencyChecker<EveryFeatureRequiresParent,
                                   CheckFeatureParentChildRelationShip>;
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
      assert(!this->isUncommited() &&
             "Transaction in CopyMode should be commited before destruction.");
    } else {
      if (this->isUncommited()) { // In modification mode we should ensure that
                                  // changes are commited before destruction
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

  decltype(auto) setRoot(std::unique_ptr<RootFeature> Root) {
    if constexpr (IsCopyMode) {
      return this->setRootImpl(std::move(Root));
    } else {
      this->setRootImpl(std::move(Root));
    }
  }

  decltype(auto)
  addChild(const std::variant<std::string, FeatureTreeNode *> &Parent,
           const std::variant<std::string, FeatureTreeNode *> &Child) {
    if constexpr (IsCopyMode) {
      return this->addChildImpl(Parent, Child);
    } else {
      this->addChildImpl(Parent, Child);
    }
  }

  decltype(auto)
  setParent(const std::variant<std::string, FeatureTreeNode *> &Child,
            const std::variant<std::string, FeatureTreeNode *> &Parent) {
    if constexpr (IsCopyMode) {
      return this->setParentImpl(Child, Parent);
    } else {
      this->setParentImpl(Child, Parent);
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
void addFeature(FeatureModel *FM, std::unique_ptr<Feature> NewFeature,
                Feature *Parent = nullptr);

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
  static void setParent(FeatureTreeNode &F, FeatureTreeNode *Parent) {
    F.setParent(Parent);
  }

  /// \brief Remove the parent of a \a Feature.
  static void removeParent(FeatureTreeNode &F) { F.setParent(nullptr); }

  /// \brief Add a \a Feature Child to F.
  static void addChild(FeatureTreeNode &F, FeatureTreeNode &Child) {
    F.addEdge(&Child);
  }

  /// \brief Remove \a Feature Child from F.
  static void removeChild(FeatureTreeNode &F, FeatureTreeNode &Child) {
    F.removeEdge(&Child);
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
      setParent(*InsertedFeature, Parent);
      addChild(*Parent, *InsertedFeature);
    } else {
      assert(FM.getRoot());
      setParent(*InsertedFeature, FM.getRoot());
      addChild(*FM.getRoot(), *InsertedFeature);
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
          setParent(*C, NewRoot);
          addChild(*NewRoot, *C);
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

class AddChild : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    addChild(std::holds_alternative<std::string>(Parent)
                 ? *FM.getFeature(std::get<std::string>(Parent))
                 : *std::get<FeatureTreeNode *>(Parent),
             std::holds_alternative<std::string>(Child)
                 ? *FM.getFeature(std::get<std::string>(Child))
                 : *std::get<FeatureTreeNode *>(Child));
  }

private:
  AddChild(std::variant<std::string, FeatureTreeNode *> Parent,
           std::variant<std::string, FeatureTreeNode *> Child)
      : Child(std::move(Child)), Parent(std::move(Parent)) {}

  std::variant<std::string, FeatureTreeNode *> Child;
  std::variant<std::string, FeatureTreeNode *> Parent;
};

class SetParent : public FeatureModelModification {
  friend class FeatureModelModification;

public:
  void exec(FeatureModel &FM) override { (*this)(FM); }

  void operator()(FeatureModel &FM) {
    setParent(std::holds_alternative<std::string>(Child)
                  ? *FM.getFeature(std::get<std::string>(Child))
                  : *std::get<FeatureTreeNode *>(Child),
              std::holds_alternative<std::string>(Parent)
                  ? FM.getFeature(std::get<std::string>(Parent))
                  : std::get<FeatureTreeNode *>(Parent));
  }

private:
  SetParent(std::variant<std::string, FeatureTreeNode *> Child,
            std::variant<std::string, FeatureTreeNode *> Parent)
      : Child(std::move(Child)), Parent(std::move(Parent)) {}

  std::variant<std::string, FeatureTreeNode *> Child;
  std::variant<std::string, FeatureTreeNode *> Parent;
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

  [[nodiscard]] inline bool isUncommited() const { return FM != nullptr; }

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
      Feature *TranslatedParent = FM->getFeature(Parent->getName());
      return FeatureModelModification::make_modification<AddFeatureToModel>(
          std::move(NewFeature), TranslatedParent)(*FM);
    }

    return FeatureModelModification::make_modification<AddFeatureToModel>(
        std::move(NewFeature))(*FM);
  }

  RootFeature *setRootImpl(std::unique_ptr<RootFeature> Root) {
    if (!FM) {
      return nullptr;
    }

    return FeatureModelModification::make_modification<SetRoot>(
        std::move(Root))(*FM);
  }

  static void
  addChildImpl(const std::variant<std::string, FeatureTreeNode *> &Parent,
               const std::variant<std::string, FeatureTreeNode *> &Child) {
    FeatureModelModification::make_modification<AddChild>(Parent, Child);
  }

  static void
  setParentImpl(const std::variant<std::string, FeatureTreeNode *> &Child,
                const std::variant<std::string, FeatureTreeNode *> &Parent) {
    FeatureModelModification::make_modification<SetParent>(Child, Parent);
  }

private:
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

  [[nodiscard]] inline bool isUncommited() const { return FM != nullptr; }

  //===--------------------------------------------------------------------===//
  // Modifications

  void addFeatureImpl(std::unique_ptr<Feature> NewFeature, Feature *Parent) {
    assert(FM && "");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<AddFeatureToModel>(
            std::move(NewFeature), Parent));
  }

  void setRootImpl(std::unique_ptr<RootFeature> Root) {
    assert(FM && "");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetRoot>(
            std::move(Root)));
  }

  void addChildImpl(const std::variant<std::string, FeatureTreeNode *> &Parent,
                    const std::variant<std::string, FeatureTreeNode *> &Child) {
    assert(FM && "");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<AddChild>(Parent,
                                                                     Child));
  }

  void
  setParentImpl(const std::variant<std::string, FeatureTreeNode *> &Child,
                const std::variant<std::string, FeatureTreeNode *> &Parent) {
    assert(FM && "");

    Modifications.push_back(
        FeatureModelModification::make_unique_modification<SetParent>(Child,
                                                                      Parent));
  }

private:
  FeatureModel *FM;
  std::vector<std::unique_ptr<FeatureModelModification>> Modifications;
};

} // namespace detail

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELTRANSACTION_H
