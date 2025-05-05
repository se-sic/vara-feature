#ifndef VARA_FEATURE_FEATUREEDGE_H
#define VARA_FEATURE_FEATUREEDGE_H

#include <QGraphicsItem>

class FeatureNode;

class FeatureEdge : public QGraphicsItem {
public:
  FeatureEdge(FeatureNode *SourceNode, FeatureNode *TargetNode);

  [[nodiscard]] FeatureNode *sourceNode() const;
  [[nodiscard]] FeatureNode *targetNode() const;
  void setSourceNode(FeatureNode *Node);
  void adjust();
  enum { Type = UserType + 2 };
  [[nodiscard]] int type() const override { return Type; }

protected:
  QRectF boundingRect() const override;
  void paint(QPainter *Painter, const QStyleOptionGraphicsItem *Option,
             QWidget *Widget) override;

private:
  FeatureNode *Source, *Target;

  QPointF SourcePoint;
  QPointF TargetPoint;
  qreal ArrowSize = 10;
};

#endif // VARA_FEATURE_FEATUREEDGE_H
