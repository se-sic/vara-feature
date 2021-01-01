#ifndef VARA_FEATURE_FEATUREMODELTRANSACTION_H
#define VARA_FEATURE_FEATUREMODELTRANSACTION_H

#include "vara/Feature/FeatureModel.h"

#include "llvm/ADT/StringRef.h"

#include <algorithm>
#include <memory>
#include <tuple>
#include <type_traits>

namespace vara::feature {

namespace detail {
class CopyTransactionMode;
class ModifyTransactionMode;
class FeatureModelCopyTransactionBase;
class FeatureModelModifyTransactionBase;
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

  using ConsistencyCheck =
      FeatureModelConsistencyChecker<EveryFeatureRequiresParent,
                                     CheckFeatureParentChildRelationShip>;

public:
  /// \brief Opens a new Transaction for the given FeatureModel. The Transaction
  /// enables the user to modify the underlying FeatureModel via a specific
  /// API, preserving the correctness of the FeatureModel.
  static FeatureModelTransaction openTransaction(FeatureModel *FM) {
    // TODO: maybe return Optional<FeatureModelTransaciton>?
    assert(ConsistencyCheck::isFeatureModelValid(*FM) &&
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
  std::conditional_t<IsCopyMode, std::unique_ptr<FeatureModel>, void> commit() {
    // TODO: needs consistency check call after modifications, maybe we should
    // move this into the base classes.
    return this->commitImpl();
  }

  /// Abort the current Transaction, throwing away all changes.
  void abort() { this->abortImpl(); }

  /// \brief Add a Feature to the FeatureModel
  ///
  /// \returns a pointer to the inserted Feature in CopyMode, otherwise,
  ///          nothing.
  std::conditional_t<IsCopyMode, Feature *, void>
  addFeature(std::unique_ptr<Feature> NewFeature, Feature *Parent) {
    if constexpr (IsCopyMode) {
      return this->addFeatureImpl(std::move(NewFeature), Parent);
    } else {
      this->addFeatureImpl(std::move(NewFeature), Parent);
    }
  }

private:
  FeatureModelTransaction(FeatureModel *FM) : TransactionBaseTy(FM) {}
};

using FeatureModelCopyTransaction =
    FeatureModelTransaction<detail::CopyTransactionMode>;
using FeatureModelModifyTransaction =
    FeatureModelTransaction<detail::ModifyTransactionMode>;

//===----------------------------------------------------------------------===//
//                            Modifiction Helpers
//===----------------------------------------------------------------------===//

/// Adds a Feature to the FeatureModel
/// TODO: finish
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

struct CopyTransactionMode {};
struct ModifyTransactionMode {};

class FeatureModelModification {
  friend class FeatureModelCopyTransactionBase;
  friend class FeatureModelModifyTransactionBase;

public:
  virtual ~FeatureModelModification() = default;

  /// \brief Execute the modification on the given FeatureModel.
  virtual void exec(FeatureModel &FM) = 0;

protected:
  /// \brief Set the parrent of a Feature.
  static void setParent(Feature &F, Feature *Parent) {
    // TODO: If the Feature is already in the model we need to update all other
    // parent/child relations
    F.setParent(Parent);
  }

  /// \brief Adds a new Feature to the FeatureModel.
  ///
  /// \param FM model to add to
  /// \param NewFeature the Feature to add
  ///
  /// \returns A pointer to the inserted Feature.
  static Feature *addFeature(FeatureModel &FM,
                             std::unique_ptr<Feature> NewFeature) {
    llvm::StringRef NewFeatureName = NewFeature->getName();
    FM.addFeature(std::move(NewFeature));
    return FM.getFeature(NewFeatureName);
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
    std::string NewFeatureName = NewFeature->getName().str();
    setParent(*NewFeature, Parent);
    return addFeature(FM, std::move(NewFeature));
  }

private:
  AddFeatureToModel(std::unique_ptr<Feature> NewFeature, Feature *Parent)
      : NewFeature(std::move(NewFeature)), Parent(Parent) {}

  std::unique_ptr<Feature> NewFeature;
  Feature *Parent;
};

class FeatureModelCopyTransactionBase {
protected:
  FeatureModelCopyTransactionBase(FeatureModel *FM)
      : FM(nullptr // TODO: figure out how to correctly copy a feature model
        ) {}

  [[nodiscard]] inline std::unique_ptr<FeatureModel> commitImpl() {
    return std::move(FM);
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

    return FeatureModelModification::make_modification<AddFeatureToModel>(
        std::move(NewFeature), Parent)(*FM);
  }

private:
  std::unique_ptr<FeatureModel> FM;
};

class FeatureModelModifyTransactionBase {
protected:
  FeatureModelModifyTransactionBase(FeatureModel *FM) : FM(FM) {}

  void commitImpl() {
    assert(FM && "Cannot commit Modifications without a FeatureModel present.");
    if (FM) {
      std::for_each(
          Modifications.begin(), Modifications.end(),
          [this](const std::unique_ptr<FeatureModelModification> &FMM) {
            FMM->exec(*FM);
          });
      FM = nullptr;
    }
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

private:
  FeatureModel *FM;
  std::vector<std::unique_ptr<FeatureModelModification>> Modifications;
};

} // namespace detail

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELTRANSACTION_H
