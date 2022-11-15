//
// Created by simon on 07.11.22.
//

#include "FeatureModelEditor.h"
#include "FeatureModelGraph.h"
#include "FeatureNode.h"
#include "ui_FeatureModelEditor.h"
#include "vara/Feature/FeatureModel.h"
FeatureModelEditor::FeatureModelEditor(QWidget *Parent) : QMainWindow(Parent), Ui(new Ui::FeatureModelEditor) {
  auto Model =  vara::feature::loadFeatureModel("/home/simon/Workspace/vara-feature/tools/fm-editor/test_children.xml");
  Ui->setupUi(this);
  auto * featureModelGraph= new FeatureModelGraph(std::move(Model),Ui->centralwidget);
  Ui->featureGraph = featureModelGraph;
  Ui->featureGraph->setObjectName(QString::fromUtf8("featureGraph"));
  Ui->gridLayout->addWidget(Ui->featureGraph, 0, 0, 2, 1);
  for(auto * Node:featureModelGraph->getNodes()) {
    QObject::connect(Node,&FeatureNode::clicked, this, &FeatureModelEditor::loadFeature);
  }
}
