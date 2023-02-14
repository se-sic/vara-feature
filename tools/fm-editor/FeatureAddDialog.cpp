//
// Created by simon on 29.11.22.
//

#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
using vara::feature::FeatureModel;
FeatureAddDialog::FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent)
    : QDialog(Parent){
  setupUi(this);
  NodeNames = QStringList();
  for(const auto &Node: *Graph->getNodes()){
    NodeNames.push_back(Node->getQName());
  }
  const auto ConstNodeNames = NodeNames;
  this->Nodes->addItems(ConstNodeNames);
  this->FeatureKind->addItems(QStringList{"Binary","Numeric","Root","Unknown"});
  NumericFeature->setVisible(false);
  connect(FeatureKind,&QComboBox::activated, this,
          &FeatureAddDialog::featureType);
}

QString FeatureAddDialog::getName() {
  return name->text();
}

std::string FeatureAddDialog::getParent(){
  return Nodes->currentText().toStdString();
}

void FeatureAddDialog::featureType(int index) {
  if(index ==1) {
    NumericFeature->setVisible(true);
  }else{
    NumericFeature->setVisible(false);
  }
}
vara::feature::Feature::FeatureKind FeatureAddDialog::getFeatureKind() {
  return vara::feature::Feature::FeatureKind(FeatureKind->currentIndex());
}
