#ifndef VARA_FEATURE_FEATURETREEVIEWMODEL_H
#define VARA_FEATURE_FEATURETREEVIEWMODEL_H

#include "FeatureTreeItem.h"
#include "vara/Feature/FeatureModel.h"

#include <QAbstractItemModel>

class FeatureTreeViewModel : public QAbstractItemModel {
public:
  FeatureTreeViewModel(vara::feature::FeatureModel* Model, QObject *Parent): QAbstractItemModel(Parent), Model(Model), RootItem(FeatureTreeItem::createFeatureTreeItem(Model->getRoot())) {
    Items = RootItem->getChildrenRecursive();
    Items.push_back(RootItem);
  }
  ~FeatureTreeViewModel() override{
    std::destroy(Items.begin(), Items.end());
  }
  std::vector<FeatureTreeItem*> getItems();
  [[nodiscard]] QVariant data(const QModelIndex &Index, int Role = Qt::DisplayRole)const override;
  [[nodiscard]] int rowCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex index(int Row, int Column, const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex &Child) const override;
  [[nodiscard]] int columnCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &Index) const override;
  [[nodiscard]] QVariant headerData(int Section, Qt::Orientation Orientation, int Role = Qt::DisplayRole) const override;
  FeatureTreeItem* addFeature(vara::feature::Feature* Item,string Parent);
  void deleteFeatureItem(bool Recursive,vara::feature::Feature* Feature);
  void deleteItem(bool Recursive, FeatureTreeItem* Item);
  FeatureTreeItem* getItem(string Name) {
    auto Item = std::find_if(Items.begin(), Items.end(),[&Name](auto I){return I->getName()== Name;});
  if(Item != Items.end()) {
    return *Item;
  }

  return nullptr;
 }

private:
  vara::feature::FeatureModel* Model;
  FeatureTreeItem* RootItem;
  std::vector<FeatureTreeItem*> Items;
};

#endif // VARA_FEATURE_FEATURETREEVIEWMODEL_H
