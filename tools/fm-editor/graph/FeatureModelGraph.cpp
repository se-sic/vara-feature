#include "FeatureModelGraph.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"

#include <QKeyEvent>
#include <QRandomGenerator>

#include <cmath>

using vara::feature::Feature;

FeatureModelGraph::FeatureModelGraph(vara::feature::FeatureModel *FeatureModel,
                                     QWidget *Parent)
    : QGraphicsView(Parent),
      EntryNode(new FeatureNode(FeatureModel->getRoot())),
      FeatureModel(FeatureModel) {
  auto *Scene = new QGraphicsScene(this);
  Scene->setItemIndexMethod(QGraphicsScene::NoIndex);

  setScene(Scene);
  setCacheMode(CacheBackground);
  setViewportUpdateMode(BoundingRectViewportUpdate);
  setRenderHint(QPainter::Antialiasing);
  setTransformationAnchor(AnchorUnderMouse);
  scale(qreal(0.8), qreal(0.8));
  reload();
  Scene->setSceneRect(0, 0, EntryNode->childrenWidth() + 100,
                      100 * EntryNode->childrenDepth() + 100);
}

void FeatureModelGraph::reload() {
  Nodes.push_back(std::unique_ptr<FeatureNode>(EntryNode));
  auto *Scene = this->scene();
  Scene->clear();
  Scene->addItem(EntryNode);
  buildRec(EntryNode);
  auto NextChildren = std::vector<FeatureNode *>(EntryNode->children().size());
  auto CurrentChildren = EntryNode->children();
  std::transform(CurrentChildren.begin(), CurrentChildren.end(),
                 NextChildren.begin(),
                 [](FeatureEdge *Edge) { return Edge->targetNode(); });
  positionRec(1, NextChildren, 5);
  EntryNode->setPos(EntryNode->childrenWidth() / 2, 10);
}

void FeatureModelGraph::buildRec(FeatureNode *CurrentFeatureNode) {
  for (auto *Feature :
       CurrentFeatureNode->getFeature()->getChildren<vara::feature::Feature>(
           1)) {
    auto Node = std::make_unique<FeatureNode>(Feature);
    auto *Edge = new FeatureEdge(CurrentFeatureNode, Node.get());
    scene()->addItem(Edge);
    scene()->addItem(Node.get());
    buildRec(Node.get());
    Nodes.push_back(std::move(Node));
  }
  for (auto *Relation : CurrentFeatureNode->getFeature()
                            ->getChildren<vara::feature::Relationship>(1)) {
    for (auto *Feature : Relation->getChildren<vara::feature::Feature>(1)) {
      auto Node = std::make_unique<FeatureNode>(Feature);
      auto *Edge = new FeatureEdge(CurrentFeatureNode, Node.get());
      scene()->addItem(Edge);
      scene()->addItem(Node.get());
      buildRec(Node.get());
      Nodes.push_back(std::move(Node));
    }
  }
}

int FeatureModelGraph::positionRec(const int CurrentDepth,
                                   const std::vector<FeatureNode *> &Children,
                                   const unsigned long Offset) {
  if (Children.empty()) {
    return CurrentDepth - 1;
  }
  int MaxDepth = CurrentDepth;
  auto NextOffset = Offset;
  for (FeatureNode *Node : Children) {
    auto NextChildren = std::vector<FeatureNode *>(Node->children().size());
    auto CurrentChildren = Node->children();
    std::transform(CurrentChildren.begin(), CurrentChildren.end(),
                   NextChildren.begin(),
                   [](FeatureEdge *Edge) { return Edge->targetNode(); });
    int const Depth = positionRec(CurrentDepth + 1, NextChildren, NextOffset);
    int Width = Node->childrenWidth() + 5;
    Node->setPos(NextOffset + Width / 2, 100 * CurrentDepth);
    NextOffset += Width;
    MaxDepth = MaxDepth < Depth ? Depth : MaxDepth;
  }

  return MaxDepth;
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

FeatureNode *FeatureModelGraph::addNode(Feature *Feature, FeatureNode *Parent) {
  auto NewNode = std::make_unique<FeatureNode>(Feature);
  auto *NewEdge = new FeatureEdge(Parent, NewNode.get());
  scene()->addItem(NewEdge);
  scene()->addItem(NewNode.get());
  auto NewNodeRaw = NewNode.get();
  Nodes.push_back(std::move(NewNode));
  auto NextChildren = std::vector<FeatureNode *>(EntryNode->children().size());
  auto CurrentChildren = EntryNode->children();
  std::transform(CurrentChildren.begin(), CurrentChildren.end(),
                 NextChildren.begin(),
                 [](FeatureEdge *Edge) { return Edge->targetNode(); });
  positionRec(1, NextChildren, 5);
  EntryNode->setPos(EntryNode->childrenWidth() / 2, 10);
  return NewNodeRaw;
}

FeatureNode *FeatureModelGraph::getNode(std::string Name) {
  auto It = std::find_if(Nodes.begin(), Nodes.end(), [&Name](auto const &Node) {
    return Node->getName() == Name;
  });
  if (It != Nodes.end()) {
    return It->get();
  }

  return nullptr;
}

void FeatureModelGraph::deleteNode(bool Recursive, FeatureNode *Node) {
  auto *Parent = Node->parent()->sourceNode();
  if (!Recursive) {
    for (auto *Child : Node->children()) {
      Child->setSourceNode(Parent);
    }

    Node->children().clear();
  } else {
    for (auto *Child : Node->children()) {
      deleteNode(true, Child->targetNode());
    }
  }
  Parent->removeChild(Node);
  scene()->removeItem(Node);
  scene()->removeItem(Node->parent());

  Nodes.erase(std::find_if(Nodes.begin(), Nodes.end(),
                           [Node](auto &N) { return N.get() == Node; }));
}

void FeatureModelGraph::deleteNode(bool Recursive,
                                   vara::feature::Feature *Feature) {
  auto *Node = getNode(Feature->getName().str());
  deleteNode(Recursive, Node);
}
