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
  Ui->textEdit->setReadOnly(true);
  auto *Highliter = new QSourceHighlite::QSourceHighliter(Ui->textEdit->document());
  Highliter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeCpp);
  QObject::connect(Ui->loadModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::loadGraph);
  QObject::connect(Ui->findModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::findModel);
  QObject::connect(Ui->actionAddFeature,&QAction::triggered, this,
                   &FeatureModelEditor::featureAddDialog);

  connect(Ui->addSource,&QPushButton::pressed, this,&FeatureModelEditor::addSource);
  connect(Ui->addSourceFile,&QPushButton::pressed, this,&FeatureModelEditor::addSourceFile);
}
void FeatureModelEditor::loadFeature(const vara::feature::Feature *Feature) {
  auto FeatureString =
      Feature->toString();
  Ui->featureInfo->setText(QString::fromStdString(FeatureString));
}
void FeatureModelEditor::loadFeaturefromIndex(const QModelIndex &Index) {
  if(Index.isValid()){
      auto Item =
          static_cast<FeatureTreeItem *>(Index.internalPointer());
      if(Item->getKind()  == vara::feature::FeatureTreeNode::NodeKind::NK_FEATURE){
        loadFeature(static_cast<FeatureTreeItemFeature*>(Item)->getItem());
      }
  }
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
  //create Graph view
  Graph = new FeatureModelGraph{Model.get()};
  Ui->tabWidget->addTab(Graph,"GraphView");
  for (auto &Node : *Graph->getNodes()) {
    QObject::connect(Node.get(), &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    QObject::connect(Node.get(), &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeature);
  }
  //create Tree View
  TreeView = new QTreeView();
  TreeModel = new FeatureTreeViewModel(Model.get(),TreeView);
  for(auto Item:TreeModel->getItems()){
    QObject::connect(Item, &FeatureTreeItem::inspectSource, this,
                     &FeatureModelEditor::inspectFeature);
  }
  connect(TreeView,&QTreeView::pressed,this,&FeatureModelEditor::loadFeaturefromIndex);
  TreeView->setModel(TreeModel);
  TreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(TreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onCustomContextMenu(const QPoint &)));
  Ui->tabWidget->addTab(TreeView,"TreeView");
  connect(Ui->sources,&QComboBox::currentTextChanged, this,&FeatureModelEditor::loadSource);
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
    if(Ui->sources->count() == 1){
      loadSource(Ui->sources->itemText(0));
    } else {
      Ui->sources->setPlaceholderText("Select File");
    }
}



void FeatureModelEditor::loadSource(const QString &RelativePath){
  Ui->textEdit->clear();
  auto SourcePath = Repository + "/" + RelativePath;
  QFile File(SourcePath);
  if(File.exists()){
      File.open(QFile::ReadOnly | QFile::Text);
      QTextStream ReadFile(&File);
      Ui->textEdit->setText(ReadFile.readAll());
      QTextCharFormat Fmt;
      Fmt.setBackground(Qt::darkYellow);
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
                            Location.getEnd()->getColumnOffset());
        Cursor.setCharFormat(Fmt);
      }
  }

}
void FeatureModelEditor::onCustomContextMenu(const QPoint &Pos) {
  auto Index = TreeView->indexAt(Pos);
  if(Index.isValid()){
      FeatureTreeItem* Item = static_cast<FeatureTreeItem*>(Index.internalPointer());
      Item->contextMenu(TreeView->mapToGlobal(Pos));
  }

}
void FeatureModelEditor::addSourceFile(){
  if(!Repository.isEmpty()) {
      QString const Path = QFileDialog::getOpenFileName(
          this, tr("Select Source File"), Repository,
          tr("C Files (*.c *c++ *.h)"));
      Ui->sources->addItem(Path.sliced(Repository.length()));
  }
}

void FeatureModelEditor::addSource() {
  auto TextEdit = Ui->textEdit;
  auto Cursor = TextEdit->textCursor();
  int start = Cursor.selectionStart();
  int end = Cursor.selectionEnd();
  Cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
  int lineStart = Cursor.position();
  int lines = 1;
  auto Block = Cursor.block();
  while (Cursor.position() > Block.position()) {
      Cursor.movePosition(QTextCursor::MoveOperation::Up);
      lines++;
  }
  Block = Block.previous();
  while(Block.isValid()){
      lines+=Block.lineCount();
      Block = Block.previous();
  }
  auto Range = vara::feature::FeatureSourceRange(Ui->sources->currentText().toStdString(),vara::feature::FeatureSourceRange::FeatureSourceLocation(lines,start-lineStart+1),vara::feature::FeatureSourceRange::FeatureSourceLocation(lines,end-lineStart));
  CurrentFeature->addLocation(Range);
  loadSource(Ui->sources->currentText());
}
