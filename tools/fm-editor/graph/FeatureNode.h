#ifndef VARA_FEATURE_FEATURENODE_H
#define VARA_FEATURE_FEATURENODE_H

#include "FeatureNode.h"
#include "vara/Feature/Feature.h"

#include <QGraphicsItem>
#include <QList>
#include <QMenu>
#include <QObject>

class FeatureEdge;
class FeatureModelGraph;

class FeatureNode : public QObject, public QGraphicsItem {
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)
public:
  void removeChild(FeatureNode *Child);
  FeatureNode(vara::feature::Feature *Feature);
  [[nodiscard]] int width() const;
  void addChildEdge(FeatureEdge *Edge);
  void setParentEdge(FeatureEdge *Edge);
  [[nodiscard]] std::vector<FeatureEdge *> children();
  [[nodiscard]] FeatureEdge *parent();
  [[nodiscard]] int childrenWidth() const;
  [[nodiscard]] int childrenDepth() const;
  enum { Type = UserType + 1 };
  [[nodiscard]] int type() const override { return Type; }
  vara::feature::Feature *getFeature() { return Feature; };
  [[nodiscard]] QRectF boundingRect() const override;
  [[nodiscard]] QPainterPath shape() const override;
  void paint(QPainter *Painter, const QStyleOptionGraphicsItem *Option,
             QWidget *Widget) override;
  bool isOptional() { return Feature->isOptional(); }
  [[nodiscard]] QString getQName() const {
    return QString::fromStdString(Feature->getName().str());
  };
  [[nodiscard]] std::string getName() const {
    return Feature->getName().str();
  };
  ~FeatureNode() override {
    std::destroy(ChildEdges.begin(), ChildEdges.end());
  }

signals:
  void clicked(const vara::feature::Feature *Feature);
  void inspectSource(vara::feature::Feature *Feature);

protected:
  QVariant itemChange(GraphicsItemChange Change,
                      const QVariant &Value) override;

  void mousePressEvent(QGraphicsSceneMouseEvent *Event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *Event) override;
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *Event) override;

private:
  std::vector<FeatureEdge *> ChildEdges;
  FeatureEdge *ParentEdge = nullptr;
  vara::feature::Feature *Feature;
  std::unique_ptr<QMenu> ContextMenu;
  void inspect();
};

#endif // VARA_FEATURE_FEATURENODE_H
