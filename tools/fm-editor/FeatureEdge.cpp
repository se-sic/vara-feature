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

  QLineF Line(mapFromItem(Source, 0, 0), mapFromItem(Target, 0, 0));
  qreal Length = Line.length();

  prepareGeometryChange();

  if (Length > qreal(20.)) {
    QPointF EdgeOffset((Line.dx() * 10) / Length, (Line.dy() * 10) / Length);
    SourcePoint = Line.p1() + EdgeOffset;
    TargetPoint = Line.p2() - EdgeOffset;
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
  double Angle = std::atan2(-Line.dy(), Line.dx());
  QPointF DestArrowP1 = TargetPoint + QPointF(sin(Angle - M_PI / 3) * ArrowSize,
                                            cos(Angle - M_PI / 3) * ArrowSize);
  QPointF DestArrowP2 = TargetPoint + QPointF(sin(Angle - M_PI + M_PI / 3) * ArrowSize,
                                            cos(Angle - M_PI + M_PI / 3) * ArrowSize);

  Painter->setBrush(Qt::black);
  Painter->drawPolygon(QPolygonF() << Line.p2() << DestArrowP1 << DestArrowP2);
}
