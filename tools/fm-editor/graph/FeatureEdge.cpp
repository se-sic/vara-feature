//
// Created by simon on 04.11.22.
//

#include "FeatureEdge.h"
#include "FeatureNode.h"

#include <QPainter>
#include <QtMath>

FeatureEdge::FeatureEdge(FeatureNode *SourceNode, FeatureNode *TargetNode)
    : Source(SourceNode), Target(TargetNode) {
  setAcceptedMouseButtons(Qt::NoButton);
  Source->addChildEdge(this);
  TargetNode->setParentEdge(this);
  adjust();
}
FeatureNode *FeatureEdge::sourceNode() const { return Source; }
FeatureNode *FeatureEdge::targetNode() const { return Target; }
void FeatureEdge::adjust() {
  if (!Source || !Target) {
    return;
  }

  QLineF Line(mapFromItem(Source, 0, 10), mapFromItem(Target, 0, -10));
  qreal Length = Line.length();

  prepareGeometryChange();

  if (Length > qreal(20.)) {
    QPointF EdgeOffset((Line.dx() * 10) / Length, (Line.dy() * 10) / Length);
    SourcePoint = Line.p1();
    TargetPoint = Line.p2();
  } else {
    SourcePoint = TargetPoint = Line.p1();
  }
}
QRectF FeatureEdge::boundingRect() const {
  if (!Source || !Target) {
    return {};
  }

  qreal PenWidth = 1;
  qreal Extra = (PenWidth + ArrowSize) / 2.0;

  return QRectF(SourcePoint, QSizeF(TargetPoint.x() - SourcePoint.x(),
                                    TargetPoint.y() - SourcePoint.y()))
      .normalized()
      .adjusted(-Extra, -Extra, Extra, Extra);
}
void FeatureEdge::paint(QPainter *Painter,
                        const QStyleOptionGraphicsItem *Option,
                        QWidget *Widget) {
  if (!Source || !Target) {
    return;
}

  QLineF Line(SourcePoint, TargetPoint);
  if (qFuzzyCompare(Line.length(), qreal(0.))) {
    return;
}

  Painter->setPen(QPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  Painter->drawLine(Line);
  Painter->setBrush(QBrush(Target->isOptional()?Qt::white:Qt::black));
  Painter->drawEllipse(TargetPoint,4,4);
}
