//
// Created by simon on 07.11.22.
//

#include "FeatureModelEditor.h"
#include "FeatureAddDialog.h"
#include "fileview/syntaxHighlite/qsourcehighliter.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
#include "ui_FeatureModelEditor.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"
#include <QDir>
#include <QFileDialog>
namespace fs = std::filesystem;
using vara::feature::FeatureModel;
using Transaction = vara::feature::FeatureModelTransaction<vara::feature::detail::ModifyTransactionMode>;
using vara::feature::Feature;

FeatureModelEditor::FeatureModelEditor(QWidget *Parent)
    : QMainWindow(Parent), Ui(new Ui::FeatureModelEditor) {

  Ui->setupUi(this);
  auto *Highliter = new QSourceHighlite::QSourceHighliter(Ui->textEdit->document());
  Highliter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeCpp);
  QObject::connect(Ui->loadModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::loadGraph);
  QObject::connect(Ui->findModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::findModel);
  QObject::connect(Ui->actionAddFeature,&QAction::triggered, this,
                   &FeatureModelEditor::featureAddDialog);

}
void FeatureModelEditor::loadFeature(vara::feature::Feature *Feature) {
  auto FeatureString =
      Feature->toString();
  Ui->featureInfo->setText(QString::fromStdString(FeatureString));
}
void FeatureModelEditor::loadGraph() {
  if(!Repository.isEmpty()){
    Repository.clear();
  }
  auto Path = Ui->ModelFile->text().toStdString();
  Model = vara::feature::loadFeatureModel(Path);
  if(!Model){
    //Return if no model at Path
    return;
  }
  Graph = new FeatureModelGraph{Model.get(), Ui->centralwidget};
  Ui->featureGraph = Graph;
  Ui->featureGraph->setObjectName(QString::fromUtf8("featureGraph"));
  Ui->gridLayout_3->addWidget(Ui->featureGraph, 1, 2, 1, 1);
  for (auto &Node : *Graph->getNodes()) {
    QObject::connect(Node.get(), &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    QObject::connect(Node.get(), &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeature);
  }
}
void FeatureModelEditor::featureAddDialog() {
   FeatureAddDialog AddDialog(Graph,this);
   if(AddDialog.exec() == QDialog::Accepted){
    auto *NewNode = Graph->addFeature(AddDialog.getName(),Graph->getNode(AddDialog.getParent()));
    QObject::connect(NewNode, &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    QObject::connect(NewNode, &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeature);
   }
}
void FeatureModelEditor::findModel() {
   QString const Path = QFileDialog::getOpenFileName(this,tr("Open Model"),"/home",tr("XML files (*.xml)"));
   Ui->ModelFile->setText(Path);
}

void FeatureModelEditor::inspectFeature(vara::feature::Feature *Feature) {
    CurrentFeature = Feature;
    if(Repository.isEmpty()){
    Repository = QFileDialog::getExistingDirectory();
    }
    Ui->sources->clear();
    for(const auto& Source : Feature->getLocations()){
      Ui->sources->addItem(QString::fromStdString(Source.getPath().string()));
    }
    connect(Ui->sources,&QComboBox::textActivated, this,&FeatureModelEditor::loadSource);
}
void FeatureModelEditor::loadSource(const QString &RelativePath){
  auto SourcePath = Repository.append("/").append(RelativePath);
  std::cout << Repository.toStdString();
  QFile File(SourcePath);
  if(File.exists()){
      File.open(QFile::ReadOnly | QFile::Text);
      QTextStream ReadFile(&File);
      Ui->textEdit->setText(ReadFile.readAll());
      QTextCharFormat Fmt;
      Fmt.setBackground(Qt::yellow);
      QTextCursor Cursor(Ui->textEdit->document());
      std::cout << CurrentFeature->toString();
      std::vector<vara::feature::FeatureSourceRange> Locations{};
      std::copy_if(CurrentFeature->getLocationsBegin(),CurrentFeature->getLocationsEnd(),std::back_inserter(Locations),[&RelativePath](auto const& Loc){return RelativePath.toStdString()==Loc.getPath();});
      for (auto &Location:Locations) {
        Cursor.movePosition(QTextCursor::MoveOperation::Start,QTextCursor::MoveMode::MoveAnchor);
        Cursor.movePosition(QTextCursor::MoveOperation::Down,
                            QTextCursor::MoveMode::MoveAnchor,
                            Location.getStart()->getLineNumber() - 1);
        Cursor.movePosition(QTextCursor::MoveOperation::NextCharacter,
                            QTextCursor::MoveMode::MoveAnchor,
                            Location.getStart()->getColumnOffset()-1);
        Cursor.movePosition(QTextCursor::MoveOperation::Down,
                            QTextCursor::MoveMode::KeepAnchor,
                            Location.getEnd()->getLineNumber() -
                                Location.getStart()->getLineNumber());
        Cursor.movePosition(QTextCursor::MoveOperation::StartOfLine,
                            QTextCursor::MoveMode::KeepAnchor);
        Cursor.movePosition(QTextCursor::MoveOperation::NextCharacter,
                            QTextCursor::MoveMode::KeepAnchor,
                            Location.getEnd()->getColumnOffset()-1);
        Cursor.setCharFormat(Fmt);
      }
  }

}
