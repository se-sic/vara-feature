
#ifndef VARA_FEATURE_FEATUREMODELGRAPH_H
#define VARA_FEATURE_FEATUREMODELGRAPH_H

#include "vara/Feature/FeatureModel.h"
#include <QGraphicsView>
class FeatureNode;
class FeatureEdge;
class FeatureModelGraph : public QGraphicsView {
  Q_OBJECT

public:
  FeatureModelGraph(vara::feature::FeatureModel * FeatureModel,
                    QWidget *Parent = nullptr);
  auto getNodes() {return &Nodes;};
  void itemMoved();
  FeatureNode* getNode(std::string Name);
  FeatureNode* addFeature(const QString& Name,FeatureNode* Parent);
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
  int TimerId = 0;
  FeatureNode* EntryNode;
  int positionRec(int CurrentDepth, const std::vector<FeatureNode *>& Children,unsigned long Offset);
  const int HEIGHT = 600;
  int WIDTH = 600;
  vara::feature::FeatureModel* FeatureModel;
  std::vector<std::unique_ptr<FeatureNode>> Nodes;
};

#endif // VARA_FEATURE_FEATUREMODELGRAPH_H
