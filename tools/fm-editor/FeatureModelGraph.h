
#ifndef VARA_FEATURE_FEATUREMODELGRAPH_H
#define VARA_FEATURE_FEATUREMODELGRAPH_H

#include "vara/Feature/FeatureModel.h"
#include <QGraphicsView>
class FeatureNode;
class FeatureEdge;
class FeatureModelGraph : public QGraphicsView {
  Q_OBJECT

public:
  FeatureModelGraph(vara::feature::FeatureModel &FeatureModel,
                    QWidget *Parent = nullptr);

  void itemMoved();

public slots:
  void zoomIn();
  void zoomOut();

protected:
  void keyPressEvent(QKeyEvent *Event) override;
  void timerEvent(QTimerEvent *Event) override;
#if QT_CONFIG(wheelevent)
  void wheelEvent(QWheelEvent *Event) override;
#endif
  void drawBackground(QPainter *Painter, const QRectF &Rect) override;

  void scaleView(qreal ScaleFactor);

private:
  void buildRec(FeatureNode *CurrentFeatureNode);
  int TimerId = 0;
  std::set<std::unique_ptr<FeatureNode>> Nodes{};
  std::set<std::unique_ptr<FeatureEdge>> Edges{};
};

#endif // VARA_FEATURE_FEATUREMODELGRAPH_H
