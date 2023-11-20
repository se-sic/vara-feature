#ifndef VARA_FEATURE_FEATURETREEVIEWMODEL_H
#define VARA_FEATURE_FEATURETREEVIEWMODEL_H

#include "FeatureTreeItem.h"
#include "vara/Feature/FeatureModel.h"

#include <QAbstractItemModel>

class FeatureTreeViewModel : public QAbstractItemModel {
public:
  FeatureTreeViewModel(vara::feature::FeatureModel *Model, QObject *Parent)
      : QAbstractItemModel(Parent) {
    auto UniqueRoot = FeatureTreeItem::createFeatureTreeItem(Model->getRoot());
    RootItem = new FeatureTreeItemFeature(nullptr);
    RootItem->addChild(UniqueRoot.get());
    auto RawRoot = UniqueRoot.get();
    Items.push_back(std::move(UniqueRoot));
    buildRecursive(RawRoot);
  }
  ~FeatureTreeViewModel() override {
    delete RootItem;
    std::destroy(Items.begin(), Items.end());
  }

  std::vector<std::unique_ptr<FeatureTreeItem>> *getItems();
  [[nodiscard]] QVariant data(const QModelIndex &Index,
                              int Role = Qt::DisplayRole) const override;
  [[nodiscard]] int
  rowCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex
  index(int Row, int Column,
        const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex &Child) const override;
  [[nodiscard]] int
  columnCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &Index) const override;
  [[nodiscard]] QVariant headerData(int Section, Qt::Orientation Orientation,
                                    int Role = Qt::DisplayRole) const override;
  FeatureTreeItem *addFeature(vara::feature::Feature *Feature, string Parent);
  void deleteFeatureItem(bool Recursive, vara::feature::Feature *Feature);
  void deleteItem(bool Recursive, FeatureTreeItem *Item);
  FeatureTreeItem *getItem(string Name) {
    auto Item =
        std::find_if(Items.begin(), Items.end(),
                     [&Name](const auto &I) { return I->getName() == Name; });
    if (Item != Items.end()) {
      return Item->get();
    }

    return nullptr;
  }

private:
  void buildRecursive(FeatureTreeItem *Parent) {
    for (auto *ChildItem : Parent->getItem()->children()) {
      FeatureTreeItem *RawChild;
      if (vara::feature::Relationship::classof(ChildItem)) {
        auto Child = FeatureTreeItem::createFeatureTreeItem(
            dynamic_cast<vara::feature::Relationship *>(ChildItem));
        Parent->addChild(Child.get());
        Child->setParent(Parent);
        RawChild = Child.get();
        Items.push_back(std::move(Child));
      } else {
        auto Child = FeatureTreeItem::createFeatureTreeItem(
            llvm::dyn_cast<vara::feature::Feature>(ChildItem));
        Parent->addChild(Child.get());
        Child->setParent(Parent);
        RawChild = Child.get();
        Items.push_back(std::move(Child));
      }
      buildRecursive(RawChild);
    }
  }
  FeatureTreeItem *RootItem;
  std::vector<std::unique_ptr<FeatureTreeItem>> Items;
};

#endif // VARA_FEATURE_FEATURETREEVIEWMODEL_H
