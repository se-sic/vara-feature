//
// Created by simon on 04.11.22.
//

#include "FeatureNode.h"
#include "FeatureEdge.h"
#include "FeatureModelGraph.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QMenu>

FeatureNode::FeatureNode(FeatureModelGraph *Graph, vara::feature::Feature *Feature) : Graph(Graph),Feature(Feature) {
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
}

void FeatureNode::addChildEdge(FeatureEdge *Edge) {
  ChildEdges.push_back(Edge);
  Edge->adjust();
}
void FeatureNode::setParentEdge(FeatureEdge *Edge) {
  ParentEdge = Edge;
  Edge->adjust();
}

std::vector<FeatureEdge *> FeatureNode::children() const {
  return ChildEdges;
}

FeatureEdge * FeatureNode::parent() const {
  return ParentEdge;
}

QRectF FeatureNode::boundingRect() const {
  qreal Adjust = 2;
  int W = width();
  return {-W / 2 - Adjust, -10 - Adjust, W + Adjust, 23 + Adjust};
}

QPainterPath FeatureNode::shape() const {
  QPainterPath Path;
  int W = width();
  Path.addRect(-W / 2, -10, W, 20);
  return Path;
}

void FeatureNode::paint(QPainter *Painter,
                        const QStyleOptionGraphicsItem *Option,
                        QWidget *Widget) {
  const auto Name = getQName();
  QBrush Brush(Qt::darkYellow);
  if (Option->state & QStyle::State_Sunken) {
    Brush.setColor(QColor(Qt::yellow).lighter(120));
  } else {
  }
  Painter->setBrush(Brush);

  Painter->setPen(QPen(Qt::black, 0));
  int W = width();
  Painter->drawRect(-W / 2, -10, W, 20);
  Painter->setPen(QPen(Qt::black, 1));
  Painter->drawText(-W/2+5, 5, Name);
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
    Graph->itemMoved();
    break;
  default:
    break;
  }

  return QGraphicsItem::itemChange(Change, Value);
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
  auto *Menu = new QMenu;
  auto *Inspect = new QAction("Inspect Sources", this);
  Menu->addAction(Inspect);
  Menu->popup(Event->screenPos());
  connect(Inspect, &QAction::triggered,
          this, &FeatureNode::inspect);
}
void FeatureNode::inspect() {
  emit(inspectSource(Feature));
}

int FeatureNode::width() const {
  auto Name = Feature->getName();
  return 15 + 5 * Name.str().length();
}

int FeatureNode::childrenWidth() const {
  if(children().empty()){
    return width();
  }
  int Result = 0;
    for (auto *Child : children()){
    Result +=Child->targetNode()->childrenWidth();
    }
    return std::max(Result,width());
}

int FeatureNode::childrenDepth() const{
  if(children().empty()){
    return 1;
  }
  int MaxDepth = 0;
    for(auto *Child: children()){
      int const ChildDepth = Child->targetNode()->childrenDepth();
      if(ChildDepth +1> MaxDepth){
      MaxDepth = ChildDepth +1;
      }
    }
    return MaxDepth;
}
