#ifndef VARA_FEATURE_FEATUREMODELTRANSACTION_H
#define VARA_FEATURE_FEATUREMODELTRANSACTION_H

#include "vara/Feature/FeatureModel.h"
#include "llvm/ADT/StringRef.h"

#include <algorithm>
#include <memory>
#include <type_traits>

namespace vara::feature {

namespace detail {

struct CopyTransactionMode {};
struct ModifyTransactionMode {};

class FeatureModelModification {
public:
  virtual ~FeatureModelModification() = default;

  virtual void exec(FeatureModel &FM) = 0;

protected:
  static void setParent(Feature &F, Feature *Parent) { F.setParent(Parent); }
  static Feature *addFeature(FeatureModel &FM,
                             std::unique_ptr<Feature> NewFeature) {
    llvm::StringRef NewFeatureName = NewFeature->getName();
    FM.addFeature(std::move(NewFeature));
    return FM.getFeature(NewFeatureName);
  }
};

class AddFeatureToModel : public FeatureModelModification {
public:
  AddFeatureToModel(std::unique_ptr<Feature> NewFeature, Feature *Parent)
      : NewFeature(std::move(NewFeature)), Parent(Parent) {}

  void exec(FeatureModel &FM) override { (*this)(FM); }

  Feature *operator()(FeatureModel &FM) {
    std::string NewFeatureName = NewFeature->getName().str();
    setParent(*NewFeature, Parent);
    return addFeature(FM, std::move(NewFeature));
  }

private:
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

    return AddFeatureToModel(std::move(NewFeature), Parent)(*FM);
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
        std::make_unique<AddFeatureToModel>(std::move(NewFeature), Parent));
  }

private:
  FeatureModel *FM;
  std::vector<std::unique_ptr<FeatureModelModification>> Modifications;
};

} // namespace detail

template <typename CopyMode>
class FeatureModelTransaction
    : private std::conditional_t<
          std::is_same_v<CopyMode, detail::CopyTransactionMode>,
          detail::FeatureModelCopyTransactionBase,
          detail::FeatureModelModifyTransactionBase> {
  static inline constexpr bool IsCopyMode =
      std::is_same<CopyMode, detail::CopyTransactionMode>::value;
  using TransactionBaseTy =
      std::conditional_t<IsCopyMode, detail::FeatureModelCopyTransactionBase,
                         detail::FeatureModelModifyTransactionBase>;

public:
  static FeatureModelTransaction openTransaction(FeatureModel *FM) {
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
      if (this->isUncommited()) {
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
    return this->commitImpl();
  }

  /// Abort the current Transaction, throwing away all changes.
  void abort() { this->abortImpl(); }

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

} // namespace vara::feature

#endif // VARA_FEATURE_FEATUREMODELTRANSACTION_H
