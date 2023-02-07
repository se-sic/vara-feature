
#ifndef VARA_FEATURE_FEATURENODE_H
#define VARA_FEATURE_FEATURENODE_H

#include "vara/Feature/Feature.h"
#include <QGraphicsItem>
#include <QList>
#include <QObject>
class FeatureEdge;
class FeatureModelGraph;
class FeatureNode : public QObject,public QGraphicsItem{
  Q_OBJECT


public:
  FeatureNode(FeatureModelGraph *Graph, vara::feature::Feature *Feature);
  [[nodiscard]] int width() const;
  void addChildEdge(FeatureEdge *Edge);
  void setParentEdge(FeatureEdge *Edge);
  [[nodiscard]] std::vector<FeatureEdge *> children() const;
  [[nodiscard]] FeatureEdge * parent() const;
  [[nodiscard]] int childrenWidth() const;
  [[nodiscard]] int childrenDepth() const;
  enum { Type = UserType + 1 };
  [[nodiscard]] int type() const override { return Type; }
  vara::feature::Feature* getFeature(){return Feature;};
  [[nodiscard]] QRectF boundingRect() const override;
  [[nodiscard]] QPainterPath shape() const override;
  void paint(QPainter *Painter, const QStyleOptionGraphicsItem *Option, QWidget *Widget) override;
  bool isOptional() {return Feature->isOptional();}
  [[nodiscard]] QString getQName() const {
    return QString::fromStdString(Feature->getName().str());
  };
  [[nodiscard]] std::string getName() const {return Feature->getName().str();};
signals:
  void clicked(vara::feature::Feature *Feature);
  void inspectSource(vara::feature::Feature *Feature);
protected:
  QVariant itemChange(GraphicsItemChange Change, const QVariant &Value) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *Event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *Event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *Event) override;
private:
  std::vector<FeatureEdge *> ChildEdges;
  FeatureEdge * ParentEdge = nullptr;
  FeatureModelGraph *Graph;
  vara::feature::Feature *Feature;

  void inspect();
};

#endif // VARA_FEATURE_FEATURENODE_H