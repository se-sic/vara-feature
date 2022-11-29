//
// Created by simon on 07.11.22.
//

#include "FeatureModelEditor.h"
#include "FeatureModelGraph.h"
#include "FeatureNode.h"
#include "ui_FeatureModelEditor.h"
#include "vara/Feature/FeatureModel.h"

FeatureModelEditor::FeatureModelEditor(QWidget *Parent)
    : QMainWindow(Parent), Ui(new Ui::FeatureModelEditor) {

  Ui->setupUi(this);
  QObject::connect(Ui->loadModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::loadGraph);
}
void FeatureModelEditor::loadFeature(vara::feature::Feature *Feature) {
  auto FeatureString =
      "Name: " + Feature->getName().str() + "\nOptional: " + (Feature->isOptional()
          ? "True"
          : "False");
  Ui->featureInfo->setText(QString::fromStdString(FeatureString));
}
void FeatureModelEditor::loadGraph() {
  auto Path = Ui->ModelFile->text().toStdString();
  auto Model = vara::feature::loadFeatureModel(Path);
  auto *Graph = new FeatureModelGraph{std::move(Model), Ui->centralwidget};
  Ui->featureGraph = Graph;
  Ui->featureGraph->setObjectName(QString::fromUtf8("featureGraph"));
  Ui->gridLayout_3->addWidget(Ui->featureGraph, 1, 2, 1, 1);
  for (auto *Node : Graph->getNodes()) {
    QObject::connect(Node, &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
  }
}
