#include "FeatureNode.h"
#include "FeatureEdge.h"
#include "FeatureModelGraph.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>

FeatureNode::FeatureNode(vara::feature::Feature *Feature) : Feature(Feature) {
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
  ContextMenu = std::make_unique<QMenu>();
  ContextMenu->addAction("Inspect Sources", this, &FeatureNode::inspect);
}

void FeatureNode::addChildEdge(FeatureEdge *Edge) {
  ChildEdges.push_back(Edge);
  Edge->adjust();
}

void FeatureNode::setParentEdge(FeatureEdge *Edge) {
  ParentEdge = Edge;
  Edge->adjust();
}

std::vector<FeatureEdge *> FeatureNode::children() { return ChildEdges; }

FeatureEdge *FeatureNode::parent() { return ParentEdge; }

QRectF FeatureNode::boundingRect() const {

  const qreal Adjust = 2;
  const int W = width();
  return {-(W + widthAdjust) / 2.0 - Adjust, -10 - Adjust,
          W + widthAdjust + Adjust, 23 + 2 * Adjust};
}

QPainterPath FeatureNode::shape() const {
  QPainterPath Path;
  const int W = width();
  Path.addRect(-(W + widthAdjust) / 2.0, -10, W + widthAdjust, 20);
  return Path;
}

void FeatureNode::paint(QPainter *Painter,
                        const QStyleOptionGraphicsItem *Option,
                        QWidget *Widget) {
  const auto Name = getQName();
  QBrush Brush(Qt::darkYellow);
  if (Option->state & QStyle::State_Sunken) {
    Brush.setColor(QColor(Qt::yellow).lighter(120));
  }

  Painter->setBrush(Brush);
  Painter->setPen(QPen(Qt::black, 0));

  int const W = width();
  Painter->drawRect(-(W + widthAdjust) / 2, -10, W + widthAdjust, 20);
  Painter->setPen(QPen(Qt::black, 1));
  Painter->drawText(-W / 2, 5, Name);
}

QVariant FeatureNode::itemChange(QGraphicsItem::GraphicsItemChange Change,
                                 const QVariant &Value) {
  switch (Change) {
  case ItemPositionHasChanged:
    for (FeatureEdge *Edge : std::as_const(ChildEdges)) {
      Edge->adjust();
    }
    if (ParentEdge) {
      ParentEdge->adjust();
    }
    break;
  default:
    break;
  }

  return QGraphicsItem::itemChange(Change, Value);
}

void FeatureNode::removeChild(FeatureNode *Child) {
  auto EdgePos =
      std::find_if(ChildEdges.begin(), ChildEdges.end(),
                   [Child](auto *Edge) { return Edge->targetNode() == Child; });
  if (EdgePos != ChildEdges.end()) {
    ChildEdges.erase(EdgePos);
  }
}

void FeatureNode::mousePressEvent(QGraphicsSceneMouseEvent *Event) {
  update();
  QGraphicsItem::mousePressEvent(Event);
  emit clicked(Feature);
}

void FeatureNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *Event) {
  update();
  QGraphicsItem::mouseReleaseEvent(Event);
}

void FeatureNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *Event) {
  ContextMenu->popup(Event->screenPos());
}
void FeatureNode::inspect() { emit(inspectSource(Feature)); }

int FeatureNode::width() const {
  const QFontMetrics fm((QFont()));

  return fm.boundingRect(getQName()).width();
}

int FeatureNode::childrenWidth() const {
  if (ChildEdges.empty()) {
    return width() + 1.5 * widthAdjust;
  }

  int Result = 0;
  for (auto *Child : ChildEdges) {
    Result += Child->targetNode()->childrenWidth();
    Result += widthAdjust / 2;
  }

  return std::max(Result, width() + widthAdjust);
}

int FeatureNode::childrenDepth() const {
  if (ChildEdges.empty()) {
    return 1;
  }

  int MaxDepth = 0;
  for (auto *Child : ChildEdges) {
    int const ChildDepth = Child->targetNode()->childrenDepth();
    if (ChildDepth + 1 > MaxDepth) {
      MaxDepth = ChildDepth + 1;
    }
  }

  return MaxDepth;
}
