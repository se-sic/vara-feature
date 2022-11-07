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
FeatureModelGraph::FeatureModelGraph(vara::feature::FeatureModel *FeatureModel,
                                     QWidget *Parent)
    : QGraphicsView(Parent) {
  Edges = std::set<std::unique_ptr<FeatureEdge>>{};
  Nodes = std::set<std::unique_ptr<FeatureNode>>{};
  auto *Scene = new QGraphicsScene(this);
  Scene->setItemIndexMethod(QGraphicsScene::NoIndex);
  Scene->setSceneRect(-200, -200, 400, 400);
  setScene(Scene);
  setCacheMode(CacheBackground);
  setViewportUpdateMode(BoundingRectViewportUpdate);
  setRenderHint(QPainter::Antialiasing);
  setTransformationAnchor(AnchorUnderMouse);
  scale(qreal(0.8), qreal(0.8));
  setMinimumSize(400, 400);
  setWindowTitle(tr("Elastic Nodes"));
  auto EntryNodeUnique = std::make_unique<FeatureNode>(this, FeatureModel->getRoot());
  EntryNode = EntryNodeUnique.get();
  Scene->addItem(EntryNode);
  buildRec(EntryNode);
  Nodes.insert(std::move(EntryNodeUnique));
  auto NextChildren =std::vector<FeatureNode*>(EntryNode->children().size());
  auto CurrentChildren = EntryNode->children();
  std::transform(CurrentChildren.begin(),CurrentChildren.end(),NextChildren.begin(),[](FeatureEdge* Edge){return Edge->targetNode();});
  positionRec(1,NextChildren,WIDTH,0);
  EntryNode->setPos(WIDTH/2,0);
}

void FeatureModelGraph::buildRec(FeatureNode *CurrentFeatureNode) {
  for (auto *Feature :
       CurrentFeatureNode->getFeature()->getChildren<vara::feature::Feature>(
           1)) {
    auto Node = std::make_unique<FeatureNode>(this, Feature);
    auto Edge = std::make_unique<FeatureEdge>(CurrentFeatureNode, Node.get());
    scene()->addItem(Edge.get());
    scene()->addItem(Node.get());
    buildRec(Node.get());
    Nodes.insert(std::move(Node));
    Edges.insert(std::move(Edge));
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
    Node->setPos(NextOffset+ContainerSize/2,(HEIGHT/Depth)*CurrentDepth);
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
void FeatureModelGraph::timerEvent(QTimerEvent *Event) {
  Q_UNUSED(Event);

  for (const std::unique_ptr<FeatureNode> &Node : Nodes) {
    Node->calculateForces();
  }

  bool ItemsMoved = false;
  for (const std::unique_ptr<FeatureNode> &Node : Nodes) {
    if (Node->advancePosition()) {
      ItemsMoved = true;
    }
  }

  if (!ItemsMoved) {
    killTimer(TimerId);
    TimerId = 0;
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

  // Text
  QRectF TextRect(SceneRect.left() + 4, SceneRect.top() + 4,
                  SceneRect.width() - 4, SceneRect.height() - 4);
  QString Message(tr("Click and drag the nodes around, and zoom with the mouse "
                     "wheel or the '+' and '-' keys"));

  QFont Font = Painter->font();
  Font.setBold(true);
  Font.setPointSize(14);
  Painter->setFont(Font);
  Painter->setPen(Qt::lightGray);
  Painter->drawText(TextRect.translated(2, 2), Message);
  Painter->setPen(Qt::black);
  Painter->drawText(TextRect, Message);
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
