#ifndef VARA_FEATURE_FEATURETREEITEM_H
#define VARA_FEATURE_FEATURETREEITEM_H

#include "vara/Feature/Feature.h"
#include "vara/Feature/Relationship.h"
#include ""

#include <QMenu>
#include <QPoint>
#include <QVariant>
#include <llvm/ADT/StringMap.h>
#include <vector>
class FeatureTreeItem : public QObject {
  Q_OBJECT
public:
  FeatureTreeItem *child(int Row) {
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
  FeatureTreeItem static *
  createFeatureTreeItem(vara::feature::FeatureTreeNode *Item);
  static bool booleanColumn(int Column) { return false; }
  virtual void contextMenu(QPoint Pos) = 0;
  vara::feature::FeatureTreeNode::NodeKind getKind() { return Kind; }
  virtual string getName() { return ""; };
  void setParent(FeatureTreeItem *ParentItem) { this->Parent = ParentItem; }
signals:
  void inspectSource(vara::feature::Feature *Feature);
  void addChildFeature(vara::feature::Feature *Feature);
  void removeFeature(bool Recursive, vara::feature::Feature *Feature);

protected:
  FeatureTreeItem(vara::feature::FeatureTreeNode *Item,
                  vara::feature::FeatureTreeNode::NodeKind Kind)
      : Kind(Kind) {
    for (auto *Child : Item->children()) {
      if (vara::feature::Relationship::classof(Child)) {
        auto child = createFeatureTreeItem(
            dynamic_cast<vara::feature::Relationship *>(Child));
        Children.push_back(child);
        child->setParent(this);
      } else {
        auto child = createFeatureTreeItem(
            dynamic_cast<vara::feature::Feature *>(Child));
        Children.push_back(child);
        child->setParent(this);
      }
    }
  }

  FeatureTreeItem *Parent = nullptr;

  std::vector<FeatureTreeItem *> Children = {};

private:
  const vara::feature::FeatureTreeNode::NodeKind Kind;
};

class FeatureTreeItemFeature : public FeatureTreeItem {
  Q_OBJECT
public:
  virtual ~FeatureTreeItemFeature() = default;
  FeatureTreeItemFeature(vara::feature::Feature *Item)
      : FeatureTreeItem(Item,
                        vara::feature::FeatureTreeNode::NodeKind::NK_FEATURE),
        Item(Item) {
    ContextMenu = buildMenu(
        this,
        std::pair(QString("Inspect Sources"), &FeatureTreeItemFeature::inspect),
        std::pair(QString("Add Child"), &FeatureTreeItemFeature::addChild),
        std::pair(QString("Remove"), &FeatureTreeItemFeature::remove));
    // TODO Make recursive Removal work

  }
  [[nodiscard]] QVariant data(int Column) const override;
  [[nodiscard]] int columnCount() const override { return 5; }
  static bool booleanColumn(int Column) { return Column == 1; }
  void contextMenu(QPoint Pos) override;
  const vara::feature::Feature *getItem() const { return Item; }
  string getName() override { return Item->getName().str(); }
public slots:
  void inspect();
  void addChild();
  void removeRecursive();
  void remove();

private:
  vara::feature::Feature *Item;
  std::unique_ptr<QMenu> ContextMenu;
};

class FeatureTreeItemRelation : public FeatureTreeItem {
public:
  virtual ~FeatureTreeItemRelation() = default;
  FeatureTreeItemRelation(vara::feature::Relationship *Item)
      : FeatureTreeItem(
            Item, vara::feature::FeatureTreeNode::NodeKind::NK_RELATIONSHIP),
        Item(Item) {}
  [[nodiscard]] QVariant data(int Column) const override {
    if (Column == 0) {
      return QString::fromStdString(relationType());
    }
    return {};
  }
  [[nodiscard]] int columnCount() const override { return 1; }
  void contextMenu(QPoint Pos) override {}

private:
  vara::feature::Relationship *Item;
  static const vara::feature::FeatureTreeNode::NodeKind Kind =
      vara::feature::FeatureTreeNode::NodeKind::NK_RELATIONSHIP;
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
