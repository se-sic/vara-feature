//
// Created by simon on 02.02.23.
//

#include "FeatureTreeViewModel.h"

QModelIndex FeatureTreeViewModel::index(int Row, int Column, const QModelIndex &Parent) const{
  FeatureTreeItem* ParentItem;
  if (!Parent.isValid()) {
    ParentItem = RootItem;
  } else {
    ParentItem = static_cast<FeatureTreeItem*>(Parent.internalPointer());
  }
  auto *ChildItem= ParentItem->child(Row);
  if(ChildItem) {
    return createIndex(Row,Column,ChildItem);
  }
  return {};
}

QModelIndex FeatureTreeViewModel::parent(const QModelIndex &Child) const {
  if(!Child.isValid()){
    return {};
  }
  auto* ChildItem = static_cast<FeatureTreeItem*>(Child.internalPointer());
  auto* ParentItem = ChildItem->parent();
  if(ParentItem) {
    return createIndex(ParentItem->row(),0,ParentItem);
}
  return {};
}
int FeatureTreeViewModel::rowCount(const QModelIndex &Parent) const {
  if(Parent.column() > 0) {
    return 0;
}
  FeatureTreeItem* ParentItem;
  if (!Parent.isValid()){
    ParentItem = RootItem;
  } else {
    ParentItem = static_cast<FeatureTreeItem*>(Parent.internalPointer());
  }
  return ParentItem->childCount();
}
int FeatureTreeViewModel::columnCount(const QModelIndex &Parent) const {
  if(Parent.isValid()) {
    return static_cast<FeatureTreeItem*>(Parent.internalPointer())->columnCount();
}
  return RootItem->columnCount();
}
QVariant FeatureTreeViewModel::data(const QModelIndex &Index, int Role) const {
  if(!Index.isValid() || Role!=Qt::DisplayRole) {
  return {};
}
  auto* Item = static_cast<FeatureTreeItem* >(Index.internalPointer());
  return Item->data(Index.column());
}
Qt::ItemFlags FeatureTreeViewModel::flags(const QModelIndex &Index) const {
  if(Index.isValid()){
    auto *Item = static_cast<FeatureTreeItem*>(Index.internalPointer());
    if(Item->booleanColumn(Index.column())){
      return Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
  }
  return QAbstractItemModel::flags(Index);
}
QVariant FeatureTreeViewModel::headerData(int Section,
                                          Qt::Orientation Orientation,
                                          int Role) const {
  if(Orientation == Qt::Orientation::Horizontal && Role == Qt::DisplayRole) {
  switch (Section) {
    case 0:return QString("Feature");
    case 1:return QString("Optional");
    case 2:return QString("NumericValues");
    case 3:return QString("Locations");
    case 4:return QString("ConfigurationOption");
    default:return QString("Todo");
    }
}
return  {};
}
std::vector<FeatureTreeItem *> FeatureTreeViewModel::getItems() {
return Items;
}
FeatureTreeItem* FeatureTreeViewModel::addFeature(vara::feature::Feature *Feature, std::string Parent) {
auto Item = std::find_if(Items.begin(), Items.end(),[&Parent](auto I){return I->getName()==Parent;});
if(Item != Items.end()){
    auto NewItem = FeatureTreeItem::createFeatureTreeItem(Feature,*Item);
    (*Item)->addChild(NewItem);
    Items.push_back(NewItem);
    return NewItem;
}
return nullptr;
}
