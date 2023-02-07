//
// Created by simon on 02.02.23.
//

#ifndef VARA_FEATURE_FEATURETREEITEM_H
#define VARA_FEATURE_FEATURETREEITEM_H
#include "vara/Feature/Feature.h"
#include "vara/Feature/Relationship.h"
#include <QPoint>
#include <QVariant>
#include <vector>
class FeatureTreeItem: public QObject  {
  Q_OBJECT
public:
  virtual ~FeatureTreeItem() {
    std::destroy(Children.begin(), Children.end());
  }

  FeatureTreeItem *child(int Row) {
    if(Row<0||Row>Children.size()) {
      return nullptr;
    }
    return Children[Row];
  }
  int childCount() {
    return Children.size();
  }

  int row() {
    if(Parent) {
      auto pos =std::find(Parent->Children.begin(), Parent->Children.end(), this);
      if ( pos!=Parent->Children.end()){
        return pos-Parent->Children.begin();
      }
    }
    return 0;
  }
  FeatureTreeItem* parent() {
    return Parent;
  }
  [[nodiscard]] virtual int columnCount() const = 0;
  [[nodiscard]] virtual QVariant data(int Column) const = 0;
  FeatureTreeItem static *createFeatureTreeItem(vara::feature::Relationship* Item,FeatureTreeItem* Parent);
  FeatureTreeItem static *createFeatureTreeItem(vara::feature::Feature* Item,FeatureTreeItem* Parent);
  bool booleanColumn(int Column) {return false;}
  virtual void contextMenu(QPoint Pos) = 0;
signals:
  void inspectSource(vara::feature::Feature *Feature);
protected:
  FeatureTreeItem(vara::feature::FeatureTreeNode* Item ,FeatureTreeItem* Parent): Parent(Parent) {
    for(auto *Child : Item->children()){
      if(vara::feature::Relationship::classof(Child)) {
        Children.push_back(createFeatureTreeItem(dynamic_cast<vara::feature::Relationship*>(Child), this));
      }else {
        Children.push_back(createFeatureTreeItem(dynamic_cast<vara::feature::Feature*>(Child), this));
      }
    }
  }

private:

  FeatureTreeItem* Parent;
  std::vector<FeatureTreeItem*> Children = {};
};


class FeatureTreeItemFeature: public FeatureTreeItem{
  Q_OBJECT
public:
virtual ~FeatureTreeItemFeature() = default;
  FeatureTreeItemFeature(vara::feature::Feature* Item,FeatureTreeItem* Parent): FeatureTreeItem(Item,Parent), Item(Item) {}
  [[nodiscard]] QVariant data(int Column) const override;
  [[nodiscard]] int columnCount() const override {return 6;}
  bool booleanColumn(int Column) {return Column==1;}
  void contextMenu(QPoint Pos) override;
public slots:
  void inspect() ;
private:
  vara::feature::Feature* Item;

};


class FeatureTreeItemRelation: public FeatureTreeItem {
public:
virtual ~FeatureTreeItemRelation() = default;
FeatureTreeItemRelation(vara::feature::Relationship* Item,FeatureTreeItem* Parent): FeatureTreeItem(Item,Parent),Item(Item) {}
[[nodiscard]] QVariant data(int Column) const override{
    if(Column==0) {
        return QString::fromStdString(relationType());
}
return {};
}
[[nodiscard]] int columnCount() const override {return 1;}
void contextMenu(QPoint Pos) override{}
private:
  vara::feature::Relationship* Item;
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
