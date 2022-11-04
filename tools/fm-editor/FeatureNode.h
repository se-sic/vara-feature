
#ifndef VARA_FEATURE_FEATURENODE_H
#define VARA_FEATURE_FEATURENODE_H

#include "vara/Feature/Feature.h"
#include <QGraphicsItem>
#include <QList>

class FeatureEdge;
class FeatureModelGraph;
class FeatureNode : public QGraphicsItem {
public:
  FeatureNode(FeatureModelGraph *Parent, vara::feature::Feature *Feature);

  void addEdge(FeatureEdge *Edge);
  [[nodiscard]] QList<FeatureEdge *> edges() const;

  enum { Type = UserType + 1 };
  [[nodiscard]] int type() const override { return Type; }
  void calculateForces();
  bool advancePosition();
  vara::feature::Feature* getFeature(){return Feature;};
  [[nodiscard]] QRectF boundingRect() const override;
  [[nodiscard]] QPainterPath shape() const override;
  void paint(QPainter *Painter, const QStyleOptionGraphicsItem *Option, QWidget *Widget) override;

protected:
  QVariant itemChange(GraphicsItemChange Change, const QVariant &Value) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *Event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *Event) override;

private:
  QList<FeatureEdge *> EdgeList;
  QPointF NewPos;
  FeatureModelGraph *Parent;
  vara::feature::Feature *Feature;
};

#endif // VARA_FEATURE_FEATURENODE_H
