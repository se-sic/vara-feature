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

FeatureNode::FeatureNode(FeatureModelGraph *Parent, vara::feature::Feature *Feature) : Parent(Parent),Feature(Feature) {
  setFlag(ItemIsMovable);
  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
  setZValue(-1);
}

void FeatureNode::addEdge(FeatureEdge *Edge) {
  EdgeList << Edge;
  Edge->adjust();
}
QList<FeatureEdge *> FeatureNode::edges() const {
  return EdgeList;
}
void FeatureNode::calculateForces() {
  if (!scene() || scene()->mouseGrabberItem() == this) {
    NewPos = pos();
    return;
  }
}
bool FeatureNode::advancePosition() {
  if (NewPos == pos()) {
    return false;
}

  setPos(NewPos);
  return true;
}
QRectF FeatureNode::boundingRect() const {
  qreal Adjust = 2;
  return { -10 - Adjust, -10 - Adjust, 23 + Adjust, 23 + Adjust};
}

QPainterPath FeatureNode::shape() const
{
  QPainterPath Path;
  Path.addEllipse(-10, -10, 20, 20);
  return Path;
}
void FeatureNode::paint(QPainter *Painter,
                        const QStyleOptionGraphicsItem *Option,
                        QWidget *Widget) {
  Painter->setPen(Qt::NoPen);
  Painter->setBrush(Qt::darkGray);
  Painter->drawEllipse(-7, -7, 20, 20);

  QRadialGradient gradient(-3, -3, 10);
  if (Option->state & QStyle::State_Sunken) {
    gradient.setCenter(3, 3);
    gradient.setFocalPoint(3, 3);
    gradient.setColorAt(1, QColor(Qt::yellow).lighter(120));
    gradient.setColorAt(0, QColor(Qt::darkYellow).lighter(120));
  } else {
    gradient.setColorAt(0, Qt::yellow);
    gradient.setColorAt(1, Qt::darkYellow);
  }
  Painter->setBrush(gradient);

  Painter->setPen(QPen(Qt::black, 0));
  Painter->drawEllipse(-10, -10, 20, 20);
}
QVariant FeatureNode::itemChange(QGraphicsItem::GraphicsItemChange Change,
                                 const QVariant &Value) {
  switch (Change) {
  case ItemPositionHasChanged:
    for (FeatureEdge *Edge : std::as_const(EdgeList)) {
      Edge->adjust();
}
    Parent->itemMoved();
    break;
  default:
    break;
  };

  return QGraphicsItem::itemChange(Change, Value);
}

void FeatureNode::mousePressEvent(QGraphicsSceneMouseEvent *Event)
{
  update();
  QGraphicsItem::mousePressEvent(Event);
}

void FeatureNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *Event) {
  update();
  QGraphicsItem::mouseReleaseEvent(Event);
}
