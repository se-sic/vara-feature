//
// Created by simon on 29.11.22.
//

#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
using vara::feature::FeatureModel;
FeatureAddDialog::FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent)
    : QDialog(Parent), Graph(Graph){
  setupUi(this);
  NodeNames = QStringList();
  for(const auto &Node: *Graph->getNodes()){
    NodeNames.push_back(Node->getQName());
  }
  const auto ConstNodeNames = NodeNames;
  this->Nodes->addItems(ConstNodeNames);
}

QString FeatureAddDialog::getName() {
  return name->text();
}

std::string FeatureAddDialog::getParent(){
  return Nodes->currentText().toStdString();
}
