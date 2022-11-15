//
// Created by simon on 04.11.22.
//

#include "FeatureModelGraph.h"
#include "FeatureEdge.h"
#include "FeatureNode.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"

#include <cmath>

#include <QKeyEvent>
#include <QRandomGenerator>
FeatureModelGraph::FeatureModelGraph(std::unique_ptr<vara::feature::FeatureModel> FeatureModel,
                                     QWidget *Parent)
    : QGraphicsView(Parent), EntryNode(new FeatureNode(this, FeatureModel->getRoot())), FeatureModel(std::move(FeatureModel)) {
  auto *Scene = new QGraphicsScene(this);
  Scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  Scene->setSceneRect(0, 0, 600, 600);
  setScene(Scene);
  setCacheMode(CacheBackground);
  setViewportUpdateMode(BoundingRectViewportUpdate);
  setRenderHint(QPainter::Antialiasing);
  setTransformationAnchor(AnchorUnderMouse);
  scale(qreal(0.8), qreal(0.8));
  setMinimumSize(400, 400);
  Nodes.insert(EntryNode);
  Scene->addItem(EntryNode);
  buildRec(EntryNode);
  auto NextChildren =std::vector<FeatureNode*>(EntryNode->children().size());
  auto CurrentChildren = EntryNode->children();
  std::transform(CurrentChildren.begin(),CurrentChildren.end(),NextChildren.begin(),[](FeatureEdge* Edge){return Edge->targetNode();});
  positionRec(1,NextChildren,WIDTH-10,0);
  EntryNode->setPos(WIDTH/2,10);
}

void FeatureModelGraph::buildRec(FeatureNode *CurrentFeatureNode) {
  for (auto *Feature :
       CurrentFeatureNode->getFeature()->getChildren<vara::feature::Feature>(
           1)) {
    auto *Node = new FeatureNode(this, Feature);
    auto *Edge = new FeatureEdge(CurrentFeatureNode, Node);
    Nodes.insert(Node);
    scene()->addItem(Edge);
    scene()->addItem(Node);
    buildRec(Node);
  }
}

int FeatureModelGraph::positionRec(const int CurrentDepth, const std::vector<FeatureNode *>& Children, const unsigned long Width, const unsigned long Offset){
  if(Children.empty()){
    return CurrentDepth-1;
  }
  auto ContainerSize = Width / Children.size();
  int Container = 0;
  int MaxDepth = CurrentDepth;
  for(FeatureNode* Node : Children){
    auto NextOffset = Offset+Container*ContainerSize;
    Container++;
    auto NextChildren =std::vector<FeatureNode*>(Node->children().size());
    auto CurrentChildren = Node->children();
    std::transform(CurrentChildren.begin(),CurrentChildren.end(),NextChildren.begin(),[](FeatureEdge* Edge){return Edge->targetNode();});
    int Depth = positionRec(CurrentDepth+1,NextChildren,ContainerSize,NextOffset);
    Node->setPos(NextOffset+ContainerSize/2,(HEIGHT/(Depth+1))*CurrentDepth);
    MaxDepth = MaxDepth<Depth?Depth:MaxDepth;
  }
  return MaxDepth;
}

void FeatureModelGraph::itemMoved() {
  if (!TimerId) {
    TimerId = startTimer(1000 / 25);
  }
}
void FeatureModelGraph::keyPressEvent(QKeyEvent *Event) {
  switch (Event->key()) {
  case Qt::Key_Plus:
    zoomIn();
    break;
  case Qt::Key_Minus:
    zoomOut();
    break;
  default:
    QGraphicsView::keyPressEvent(Event);
  }
}
#if QT_CONFIG(wheelevent)
void FeatureModelGraph::wheelEvent(QWheelEvent *Event) {
  scaleView(pow(2., -Event->angleDelta().y() / 240.0));
}
#endif

void FeatureModelGraph::drawBackground(QPainter *Painter, const QRectF &Rect) {
  Q_UNUSED(Rect);

  // Shadow
  QRectF SceneRect = this->sceneRect();
  QRectF RightShadow(SceneRect.right(), SceneRect.top() + 5, 5,
                     SceneRect.height());
  QRectF BottomShadow(SceneRect.left() + 5, SceneRect.bottom(),
                      SceneRect.width(), 5);
  if (RightShadow.intersects(Rect) || RightShadow.contains(Rect)) {
    Painter->fillRect(RightShadow, Qt::darkGray);
  }
  if (BottomShadow.intersects(Rect) || BottomShadow.contains(Rect)) {
    Painter->fillRect(BottomShadow, Qt::darkGray);
  }

  // Fill
  QLinearGradient Gradient(SceneRect.topLeft(), SceneRect.bottomRight());
  Gradient.setColorAt(0, Qt::white);
  Gradient.setColorAt(1, Qt::lightGray);
  Painter->fillRect(Rect.intersected(SceneRect), Gradient);
  Painter->setBrush(Qt::NoBrush);
  Painter->drawRect(SceneRect);

  QFont Font = Painter->font();
  Font.setBold(true);
  Font.setPointSize(14);
  Painter->setFont(Font);
  Painter->setPen(Qt::lightGray);
  Painter->setPen(Qt::black);
}
void FeatureModelGraph::scaleView(qreal ScaleFactor) {
  qreal Factor = transform()
                     .scale(ScaleFactor, ScaleFactor)
                     .mapRect(QRectF(0, 0, 1, 1))
                     .width();
  if (Factor < 0.07 || Factor > 100) {
    return;
  }

  scale(ScaleFactor, ScaleFactor);
}
void FeatureModelGraph::zoomIn() { scaleView(qreal(1.2)); }
void FeatureModelGraph::zoomOut() { scaleView(1 / qreal(1.2)); }
