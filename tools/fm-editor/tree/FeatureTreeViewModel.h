//
// Created by simon on 02.02.23.
//

#ifndef VARA_FEATURE_FEATURETREEVIEWMODEL_H
#define VARA_FEATURE_FEATURETREEVIEWMODEL_H
#include "FeatureTreeItem.h"
#include "vara/Feature/FeatureModel.h"
#include <QAbstractItemModel>
class FeatureTreeViewModel : public QAbstractItemModel {
public:
  FeatureTreeViewModel(vara::feature::FeatureModel* Model, QObject *Parent): QAbstractItemModel(Parent), Model(Model), RootItem(FeatureTreeItem::createFeatureTreeItem(Model->getRoot(), nullptr)) {
    Items = RootItem->getChildrenRecursive();
  }
  ~FeatureTreeViewModel() override{
    delete RootItem;
  }
  std::vector<FeatureTreeItem*> getItems();
  [[nodiscard]] QVariant data(const QModelIndex &Index, int Role = Qt::DisplayRole)const override;
  [[nodiscard]] int rowCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex index(int Row, int Column, const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex &Child) const override;
  [[nodiscard]] int columnCount(const QModelIndex &Parent = QModelIndex()) const override;
  [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &Index) const override;
  [[nodiscard]] QVariant headerData(int Section, Qt::Orientation Orientation, int Role = Qt::DisplayRole) const override;

private:
  vara::feature::FeatureModel* Model;
  FeatureTreeItem* RootItem;
  std::vector<FeatureTreeItem*> Items;
};

#endif // VARA_FEATURE_FEATURETREEVIEWMODEL_H
