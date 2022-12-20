//
// Created by simon on 07.11.22.
//

#include "FeatureModelEditor.h"
#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
#include "ui_FeatureModelEditor.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"

using vara::feature::FeatureModel;
using Transaction = vara::feature::FeatureModelTransaction<vara::feature::detail::ModifyTransactionMode>;
using vara::feature::Feature;

FeatureModelEditor::FeatureModelEditor(QWidget *Parent)
    : QMainWindow(Parent), Ui(new Ui::FeatureModelEditor) {

  Ui->setupUi(this);
  QObject::connect(Ui->loadModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::loadGraph);
  QObject::connect(Ui->actionAddFeature,&QAction::triggered, this,
                   &FeatureModelEditor::featureAddDialog);

}
void FeatureModelEditor::loadFeature(vara::feature::Feature *Feature) {
  auto FeatureString =
      "Name: " + Feature->getName().str() + "\nOptional: " + (Feature->isOptional()
          ? "True"
          : "False") + "\nSource:";
  Ui->featureInfo->setText(QString::fromStdString(FeatureString));
}
void FeatureModelEditor::loadGraph() {
  auto Path = Ui->ModelFile->text().toStdString();
  Model = vara::feature::loadFeatureModel(Path);
  Graph = new FeatureModelGraph{Model.get(), Ui->centralwidget};
  Ui->featureGraph = Graph;
  Ui->featureGraph->setObjectName(QString::fromUtf8("featureGraph"));
  Ui->gridLayout_3->addWidget(Ui->featureGraph, 1, 2, 1, 1);
  for (auto &Node : *Graph->getNodes()) {
    QObject::connect(Node.get(), &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
  }
}
void FeatureModelEditor::featureAddDialog() {
   FeatureAddDialog AddDialog(Graph,this);
   if(AddDialog.exec() == QDialog::Accepted){
    Graph->addFeature(AddDialog.getName(),Graph->getNode(AddDialog.getParent()));
   }
}

