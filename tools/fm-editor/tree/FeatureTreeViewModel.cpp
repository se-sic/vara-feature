#include "FeatureTreeViewModel.h"

#include <utility>

QModelIndex FeatureTreeViewModel::index(int Row, int Column,
                                        const QModelIndex &Parent) const {
  FeatureTreeItem *ParentItem;

  if (!Parent.isValid()) {
    ParentItem = RootItem;
  } else {
    ParentItem = static_cast<FeatureTreeItem *>(Parent.internalPointer())
                     ->child(Parent.row());
  }

  if (ParentItem->childCount() <= 0) {
    return {};
  }

  auto *ChildItem = ParentItem->child(Row);
  if (ChildItem) {
    return createIndex(Row, Column, ParentItem);
  }

  return {};
}

QModelIndex FeatureTreeViewModel::parent(const QModelIndex &Child) const {
  if (!Child.isValid()) {
    return {};
  }

  auto *ParentItem = static_cast<FeatureTreeItem *>(Child.internalPointer());
  if (ParentItem && ParentItem != RootItem) {
    return createIndex(ParentItem->row(), 0, ParentItem->parent());
  }

  return {};
}

int FeatureTreeViewModel::rowCount(const QModelIndex &Parent) const {
  if (Parent.column() > 0) {
    return 0;
  }

  FeatureTreeItem *ParentItem;
  if (!Parent.isValid()) {
    ParentItem = RootItem;
  } else {
    ParentItem = static_cast<FeatureTreeItem *>(Parent.internalPointer())
                     ->child(Parent.row());
  }

  return ParentItem->childCount();
}

int FeatureTreeViewModel::columnCount(const QModelIndex &Parent) const {
  if (Parent.isValid()) {
    auto *Item = static_cast<FeatureTreeItem *>(Parent.internalPointer())
                     ->child(Parent.row());
    return Item->columnCount();
  }

  return RootItem->columnCount();
}

QVariant FeatureTreeViewModel::data(const QModelIndex &Index, int Role) const {
  if (!Index.isValid() || Role != Qt::DisplayRole) {
    return {};
  }

  auto *Item = static_cast<FeatureTreeItem *>(Index.internalPointer())
                   ->child(Index.row());
  return Item->data(Index.column());
}

Qt::ItemFlags FeatureTreeViewModel::flags(const QModelIndex &Index) const {
  if (Index.isValid()) {
    auto *Item = static_cast<FeatureTreeItem *>(Index.internalPointer())
                     ->child(Index.row());
    if (Item->booleanColumn(Index.column())) {
      return Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }
  }

  return QAbstractItemModel::flags(Index);
}

QVariant FeatureTreeViewModel::headerData(int Section,
                                          Qt::Orientation Orientation,
                                          int Role) const {
  if (Orientation == Qt::Orientation::Horizontal && Role == Qt::DisplayRole) {
    switch (Section) {
    case 0:
      return QString("Feature");
    case 1:
      return QString("Optional");
    case 2:
      return QString("NumericValues");
    case 3:
      return QString("Locations");
    case 4:
      return QString("ConfigurationOption");
    default:
      return QString("Todo");
    }
  }

  return {};
}

std::vector<std::unique_ptr<FeatureTreeItem>> *
FeatureTreeViewModel::getItems() {
  return &Items;
}

FeatureTreeItem *
FeatureTreeViewModel::addFeature(vara::feature::Feature *Feature,
                                 std::string Parent) {
  auto *Item = getItem(std::move(Parent));
  if (Item) {
    emit(layoutAboutToBeChanged());
    auto NewItem = FeatureTreeItem::createFeatureTreeItem(Feature);
    Item->addChild(NewItem.get());
    auto *NewItemRaw = NewItem.get();
    Items.push_back(std::move(NewItem));
    emit(layoutChanged());
    return NewItemRaw;
  }

  return nullptr;
}

void FeatureTreeViewModel::deleteFeatureItem(bool Recursive,
                                             vara::feature::Feature *Feature) {
  emit(layoutAboutToBeChanged());
  auto *Item = getItem(Feature->getName().str());
  if (Item) {
    deleteItem(Recursive, Item);
  }

  emit(layoutChanged());
}

void FeatureTreeViewModel::deleteItem(bool Recursive, FeatureTreeItem *Item) {

  if (!Recursive) {
    auto *Parent = Item->parent();
    if (Parent) {
      for (auto *Child : Item->getChildren()) {
        Parent->addChild(Child);
      }
      auto ItemPos = std::find(Parent->getChildren().begin(),
                               Parent->getChildren().end(), Item);
      Parent->getChildren().erase(ItemPos);
    }
  }

  if (Recursive) {
    for (auto *Child : Item->getChildren()) {
      deleteItem(Recursive, Child);
    }
  }

  Items.erase(std::find_if(Items.begin(), Items.end(),
                           [Item](auto &I) { return I.get() == Item; }));
}
