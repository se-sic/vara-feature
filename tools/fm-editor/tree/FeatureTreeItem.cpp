#include "FeatureTreeItem.h"

#include <QMenu>

#include <unordered_set>

QVariant numericValue(vara::feature::Feature *Item) {
  if (Item->getKind() == vara::feature::Feature::FeatureKind::FK_NUMERIC) {
    auto *NumItem = llvm::dyn_cast<vara::feature::NumericFeature>(Item);
    if (NumItem != nullptr) {
      string Result = "[";
      if (std::holds_alternative<vara::feature::NumericFeature::ValueRangeType>(
              NumItem->getValues())) {
        auto Range = std::get<vara::feature::NumericFeature::ValueRangeType>(
            NumItem->getValues());
        Result += std::to_string(Range.first) + ", " +
                  std::to_string(Range.second) + "]";
      } else {
        auto Range = std::get<vara::feature::NumericFeature::ValueListType>(
            NumItem->getValues());
        for (auto It = Range.begin(); It != Range.end(); It++) {
          if (It != Range.begin()) {
            Result += ",";
          }
          Result += std::to_string(*It.base());
        }
        Result += "]";
      }

      return QString::fromStdString(Result);
    }
  }

  return {};
}

QVariant locationString(vara::feature::Feature *Item) {
  auto Locs = Item->getLocations();
  std::stringstream StrS;
  if (Item->hasLocations()) {
    std::for_each(Locs.begin(), Locs.end(),
                  [&StrS](const vara::feature::FeatureSourceRange &Fsr) {
                    StrS << llvm::formatv("{0}; ", Fsr.toString()).str();
                  });
  }

  return QString::fromStdString(StrS.str());
}

std::unique_ptr<FeatureTreeItem>
FeatureTreeItem::createFeatureTreeItem(vara::feature::FeatureTreeNode *Item) {
  if (Item->getKind() ==
      vara::feature::FeatureTreeNode::NodeKind::NK_RELATIONSHIP) {
    return std::make_unique<FeatureTreeItemRelation>(
        llvm::dyn_cast<vara::feature::Relationship,
                       vara::feature::FeatureTreeNode>(Item));
  }

  return std::make_unique<FeatureTreeItemFeature>(
      llvm::dyn_cast<vara::feature::Feature, vara::feature::FeatureTreeNode>(
          Item));
}

void FeatureTreeItem::addChild(FeatureTreeItem *Child) {
  if (!Children.empty() && Children[0]->getKind() == ItemKind::IK_Relation) {
    Children[0]->addChild(Child);
  } else {
    Children.push_back(Child);
    Child->setParent(this);
  }
}

std::vector<FeatureTreeItem *> FeatureTreeItem::getChildrenRecursive() {
  auto Nodes = std::vector<FeatureTreeItem *>{Children};
  for (auto *Child : Children) {
    auto ChildNodes = Child->getChildrenRecursive();
    Nodes.insert(Nodes.end(), ChildNodes.begin(), ChildNodes.end());
  }

  return Nodes;
}

QVariant FeatureTreeItemFeature::data(int Column) const {
  switch (Column) {
  case 0:
    return QString::fromStdString(Item->getName().str());
  case 1:
    return Item->isOptional() ? QVariant("✓") : QVariant("x");
  case 2:
    return numericValue(Item);
  case 3:
    return locationString(Item);
  case 4:
    return QString::fromStdString(Item->getOutputString().str());
  default:
    return {};
  }
}

void FeatureTreeItemFeature::inspect() { emit(inspectSource(Item)); }

void FeatureTreeItemFeature::contextMenu(QPoint Pos) {
  ContextMenu->popup(Pos);
}

void FeatureTreeItemFeature::remove() { emit(removeFeature(false, Item)); }

void FeatureTreeItemFeature::addChild() { emit(addChildFeature(Item)); }
