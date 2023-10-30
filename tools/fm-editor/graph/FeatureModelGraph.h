#ifndef VARA_FEATURE_FEATUREMODELGRAPH_H
#define VARA_FEATURE_FEATUREMODELGRAPH_H

#include "FeatureEdge.h"
#include "FeatureNode.h"
#include "vara/Feature/FeatureModel.h"

#include <QGraphicsView>

class FeatureModelGraph : public QGraphicsView {
  Q_OBJECT

public:
  FeatureModelGraph(vara::feature::FeatureModel *FeatureModel,
                    QWidget *Parent = nullptr);
  auto getNodes() { return &Nodes; };
  FeatureNode *getNode(std::string Name);
  FeatureNode *addNode(vara::feature::Feature *Feature, FeatureNode *Parent);
  void deleteNode(bool Recursive, vara::feature::Feature *Feature);
  void deleteNode(bool Recursive, FeatureNode *Node);

public slots:
  void zoomIn();
  void zoomOut();

protected:
  void keyPressEvent(QKeyEvent *Event) override;

#if QT_CONFIG(wheelevent)
  void wheelEvent(QWheelEvent *Event) override;
#endif

  void drawBackground(QPainter *Painter, const QRectF &Rect) override;

  void scaleView(qreal ScaleFactor);

private:
  void reload();
  void buildRec(FeatureNode *CurrentFeatureNode);
  FeatureNode *EntryNode;
  int positionRec(int CurrentDepth, const std::vector<FeatureNode *> &Children,
                  unsigned long Offset);
  vara::feature::FeatureModel *FeatureModel;
  std::vector<FeatureNode *> Nodes;
  std::unique_ptr<QGraphicsScene> Scene;
};

#endif // VARA_FEATURE_FEATUREMODELGRAPH_H
