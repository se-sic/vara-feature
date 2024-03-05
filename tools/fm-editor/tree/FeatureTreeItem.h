#ifndef VARA_FEATURE_FEATURETREEITEM_H
#define VARA_FEATURE_FEATURETREEITEM_H

#include "vara/Feature/Feature.h"
#include "vara/Feature/Relationship.h"

#include <llvm/ADT/StringMap.h>

#include <QMenu>
#include <QPoint>
#include <QVariant>

#include <vector>

enum ItemKind {
  IK_Feature,
  IK_Relation,
  IK_Root

};

class FeatureTreeItem : public QObject {
  Q_OBJECT

public:
  FeatureTreeItem *child(size_t Row) {
    if (Row < 0 || Row > Children.size()) {

      return nullptr;
    }

    return Children[Row];
  }

  int childCount() { return Children.size(); }

  std::vector<FeatureTreeItem *> getChildrenRecursive();

  int row() {
    if (Parent) {
      auto pos =
          std::find(Parent->Children.begin(), Parent->Children.end(), this);
      if (pos != Parent->Children.end()) {
        return pos - Parent->Children.begin();
      }
    }

    return 0;
  }

  FeatureTreeItem *parent() { return Parent; }
  void addChild(FeatureTreeItem *Child);
  std::vector<FeatureTreeItem *> &getChildren() { return Children; }
  [[nodiscard]] virtual int columnCount() const = 0;
  [[nodiscard]] virtual QVariant data(int Column) const = 0;
  std::unique_ptr<FeatureTreeItem> static createFeatureTreeItem(
      vara::feature::FeatureTreeNode *Item);
  bool booleanColumn(int Column) { return false; };
  virtual void contextMenu(QPoint Pos) = 0;
  virtual vara::feature::FeatureTreeNode *getItem() const = 0;
  virtual ItemKind getKind() = 0;
  virtual string getName() { return ""; };
  void setParent(FeatureTreeItem *ParentItem) { this->Parent = ParentItem; }

signals:
  void inspectSource(vara::feature::Feature *Feature);
  void addChildFeature(vara::feature::Feature *Feature);
  void removeFeature(bool Recursive, vara::feature::Feature *Feature);

protected:
  FeatureTreeItem() = default;

  FeatureTreeItem *Parent = nullptr;

  std::vector<FeatureTreeItem *> Children = {};
};

class FeatureTreeItemRoot : public FeatureTreeItem {
  Q_OBJECT

public:
  FeatureTreeItemRoot(){};
  [[nodiscard]] int columnCount() const override { return 5; };
  [[nodiscard]] QVariant data(int Column) const override { return {}; };
  void contextMenu(QPoint Pos) override{};
  [[nodiscard]] vara::feature::FeatureTreeNode *getItem() const override {
    return nullptr;
  };
  ItemKind getKind() override { return IK_Root; };
};

class FeatureTreeItemFeature : public FeatureTreeItem {
  Q_OBJECT

public:
  FeatureTreeItemFeature(vara::feature::Feature *Item)
      : FeatureTreeItem(), Item(Item) {
    ContextMenu = std::make_unique<QMenu>();
    ContextMenu->addAction("Inspect Sources", this,
                           &FeatureTreeItemFeature::inspect);
    ContextMenu->addAction("Add Child", this,
                           &FeatureTreeItemFeature::addChild);
    ContextMenu->addAction("Delete", this, &FeatureTreeItemFeature::remove);
  }
  ~FeatureTreeItemFeature() override = default;

  [[nodiscard]] QVariant data(int Column) const override;
  [[nodiscard]] int columnCount() const override { return 5; }
  bool booleanColumn(int Column) { return Column == 1; }
  void contextMenu(QPoint Pos) override;
  [[nodiscard]] vara::feature::FeatureTreeNode *getItem() const override {
    return Item;
  }
  [[nodiscard]] const vara::feature::Feature *getFeature() const {
    return Item;
  }
  string getName() override { return Item->getName().str(); }
  ItemKind getKind() override { return IK_Feature; }
public slots:
  void inspect();
  void addChild();
  void remove();

private:
  vara::feature::Feature *Item;
  std::unique_ptr<QMenu> ContextMenu;
  std::unique_ptr<QAction> RemoveAction;
};

class FeatureTreeItemRelation : public FeatureTreeItem {
public:
  FeatureTreeItemRelation(vara::feature::Relationship *Item) : Item(Item){};
  ~FeatureTreeItemRelation() override = default;

  [[nodiscard]] QVariant data(int Column) const override {
    if (Column == 0) {
      return QString::fromStdString(relationType());
    }
    return {};
  }
  [[nodiscard]] int columnCount() const override { return 1; }
  void contextMenu(QPoint Pos) override {}
  [[nodiscard]] vara::feature::FeatureTreeNode *getItem() const override {
    return Item;
  }
  ItemKind getKind() override { return IK_Relation; }

private:
  vara::feature::Relationship *Item;
  [[nodiscard]] std::string relationType() const {
    std::string Type;
    switch (Item->getKind()) {

    case vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE:
      Type = "⊕ Alternative";
      break;
    case vara::feature::Relationship::RelationshipKind::RK_OR:
      Type = "+ Or";
      break;
    }
    return Type;
  }
};

#endif // VARA_FEATURE_FEATURETREEITEM_H
