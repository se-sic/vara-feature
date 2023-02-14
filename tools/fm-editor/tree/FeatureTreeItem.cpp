//
// Created by simon on 02.02.23.
//
#include "FeatureTreeItem.h"
#include <QMenu>
#include <unordered_set>
QVariant numericValue(vara::feature::Feature* Item) {
  if(Item->getKind()==vara::feature::Feature::FeatureKind::FK_NUMERIC){
    auto *NumItem = dynamic_cast<vara::feature::NumericFeature*>(Item);
    string Result ="[";
    if(std::holds_alternative<vara::feature::NumericFeature::ValueRangeType>(NumItem->getValues())){
      auto range = std::get<vara::feature::NumericFeature::ValueRangeType>(NumItem->getValues());
      Result+= std::to_string(range.first) +", "+std::to_string(range.second) + "]";
    } else {
      auto Range = std::get<vara::feature::NumericFeature::ValueListType>(NumItem->getValues());
      for(auto It = Range.begin();It!=Range.end();It++){
        if(It!=Range.begin()){
          Result+=",";
        }
        Result+=std::to_string(*It.base());
      }
      Result+="]";
    }
    return  QString::fromStdString(Result);
  }
  return {};
}


QVariant locationString(vara::feature::Feature* Item){
  auto Locs = Item->getLocations();
  std::stringstream StrS;
  if (Item->hasLocations()) {
    std::for_each(Locs.begin(), Locs.end(),
                  [&StrS](const vara::feature::FeatureSourceRange &Fsr) {
                    StrS << llvm::formatv("{0}", Fsr.toString()).str();
                  });
  }
  return QString::fromStdString(StrS.str());
}

QVariant crossTreeConstraintString(vara::feature::Feature* Item){
  std::stringstream StrS;
  for(auto Constraint:Item->constraints()){
    StrS << Constraint->toString();
  }
  return QString::fromStdString(StrS.str());
}

FeatureTreeItem*  FeatureTreeItem::createFeatureTreeItem(
    vara::feature::Relationship *Item, FeatureTreeItem *Parent)  {
  return new FeatureTreeItemRelation(Item,Parent);
}
FeatureTreeItem*  FeatureTreeItem::createFeatureTreeItem(
    vara::feature::Feature *Item, FeatureTreeItem *Parent)  {
  return new FeatureTreeItemFeature(Item, Parent);
}
void FeatureTreeItem::addChild(FeatureTreeItem* Child) {
  Children.push_back(Child);
}
std::vector<FeatureTreeItem *> FeatureTreeItem::getChildrenRecursive() {
  auto Nodes = std::vector<FeatureTreeItem*>{Children};
  for(auto Child: Children){
    auto ChildNodes = Child->getChildrenRecursive();
    Nodes.insert(Nodes.end(),ChildNodes.begin(), ChildNodes.end());
  }
  return Nodes;
}

QVariant FeatureTreeItemFeature::data(int Column) const {
  switch (Column) {
  case 0: return QString::fromStdString(Item->getName().str());
  case 1: return Item->isOptional()? QVariant("✓"):QVariant("x");
  case 2: return numericValue(Item);
  case 3: return locationString(Item);
  case 4: return crossTreeConstraintString(Item);
  case 5: return QString::fromStdString(Item->getOutputString().str());
  default:
    return {};
  }
}

void FeatureTreeItemFeature::inspect() {
  emit(inspectSource(Item));
}
void FeatureTreeItemFeature::contextMenu(QPoint Pos) {
  auto *Menu = new QMenu;
  auto *Inspect = new QAction("Inspect Sources", this);
  Menu->addAction(Inspect);
  Menu->popup(Pos);
  connect(Inspect, &QAction::triggered,
          this, &FeatureTreeItemFeature::inspect);
}
