
#ifndef VARA_FEATURE_FEATURENODE_H
#define VARA_FEATURE_FEATURENODE_H

#include "vara/Feature/Feature.h"
#include <QGraphicsItem>
#include <QList>

class FeatureEdge;
class FeatureModelGraph;
class FeatureNode : public QGraphicsItem {
public:
  FeatureNode(FeatureModelGraph *Graph, vara::feature::Feature *Feature);

  void addChildEdge(FeatureEdge *Edge);
  void setParentEdge(FeatureEdge *Edge);
  [[nodiscard]] std::vector<FeatureEdge *> children() const;
  [[nodiscard]] FeatureEdge * parent() const;

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
  std::vector<FeatureEdge *> ChildEdges;
  FeatureEdge * ParentEdge = nullptr;
  QPointF NewPos;
  FeatureModelGraph *Graph;
  vara::feature::Feature *Feature;
};

#endif // VARA_FEATURE_FEATURENODE_H
