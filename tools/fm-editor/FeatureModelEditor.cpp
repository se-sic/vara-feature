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
#include "vara/Feature/FeatureModelWriter.h"
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
  connect(Ui->actionSave,&QAction::triggered,this,&FeatureModelEditor::save);
  connect(Ui->addSource,&QPushButton::pressed, this,&FeatureModelEditor::addSource);
  connect(Ui->addSourceFile,&QPushButton::pressed, this,&FeatureModelEditor::addSourceFile);
}

///Display the information of a Feature
void FeatureModelEditor::loadFeature(const vara::feature::Feature *Feature) {
  auto FeatureString =
      Feature->toString();
  Ui->featureInfo->setText(QString::fromStdString(FeatureString));
}

///Get a Feature from an Index of the TreeView and display its information.
void FeatureModelEditor::loadFeaturefromIndex(const QModelIndex &Index) {
  if(Index.isValid()){
      auto Item =
          static_cast<FeatureTreeItem *>(Index.internalPointer());
      if(Item->getKind()  == vara::feature::FeatureTreeNode::NodeKind::NK_FEATURE){
        loadFeature(static_cast<FeatureTreeItemFeature*>(Item)->getItem());
      }
  }
}
///Clear all Fields that should be emptied when loading a new Model
void FeatureModelEditor::clean() {
  SavePath.clear();
  Repository.clear();
  Ui->tabWidget->clear();
  Ui->sources->clear();
  Ui->textEdit->clear();
  Ui->sourcesLable->setText("Sources for: ");
  Ui->featureInfo->clear();
}

///Load the Feature Model at the Path in ModelFile field and build the Views
void FeatureModelEditor::loadGraph() {
  clean();
  ModelPath = Ui->ModelFile->text();
  FeatureModel = vara::feature::loadFeatureModel(ModelPath.toStdString());
  if(!FeatureModel){
    //Return if no model at Path
    return;
  }
  //create Graph view
  buildGraph();
  // create Tree View
  buildTree();
  Ui->tabWidget->addTab(Graph.get(),"GraphView");
  Ui->tabWidget->addTab(TreeView,"TreeView");
  connect(Ui->sources,&QComboBox::currentTextChanged, this,&FeatureModelEditor::loadSource);
  Ui->actionSave->setEnabled(true);
  Ui->actionAddFeature->setEnabled(true);
}

///Build the Treeview
void FeatureModelEditor::buildTree() {
  TreeView = new QTreeView();
  TreeModel =std::make_unique<FeatureTreeViewModel>(FeatureModel.get(), TreeView);
  for(auto Item: TreeModel->getItems()){
    connect(Item, &FeatureTreeItem::inspectSource, this,
            &FeatureModelEditor::inspectFeatureSources);
  }
  connect(TreeView,&QTreeView::pressed,this,&FeatureModelEditor::loadFeaturefromIndex);
  TreeView->setModel(TreeModel.get());
  TreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(TreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(createTreeContextMenu(const QPoint &)));
}

///Build the graph view
void FeatureModelEditor::buildGraph() {
  Graph = std::make_unique<FeatureModelGraph>(FeatureModel.get());
  for (auto &Node : *Graph->getNodes()) {
    QObject::connect(Node.get(), &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    QObject::connect(Node.get(), &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeatureSources);
  }
}

///Spawn a Dialog to select data to add a Feature
void FeatureModelEditor::featureAddDialog() {
   FeatureAddDialog AddDialog(Graph.get(),this);
   if(AddDialog.exec() == QDialog::Accepted){
    Feature* Parent = FeatureModel->getFeature(AddDialog.getParent().toStdString());
    auto NewFeature = AddDialog.getFeature();

    auto *NewNode = Graph->addNode(NewFeature.get(),
                                   Graph->getNode(Parent->getName().str()));
    auto *NewTreeItem = TreeModel->addFeature(NewFeature.get(),Parent->getName().str());
    connect(NewTreeItem,&FeatureTreeItem::inspectSource, this,&FeatureModelEditor::inspectFeatureSources);
    connect(NewNode, &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    connect(NewNode, &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeatureSources);

    auto Transaction = vara::feature::FeatureModelTransaction<vara::feature::detail::ModifyTransactionMode>::openTransaction(*FeatureModel);
    Transaction.addFeature(std::move(NewFeature),Parent);
    Transaction.commit();
   }
}


///Save the current State of the Feature Model
void FeatureModelEditor::save() {
   SavePath = QFileDialog::getSaveFileName(this,tr("Save File"),ModelPath,tr("XML files (*.xml)"));
   vara::feature::FeatureModelXmlWriter FMWrite{*FeatureModel};
   FMWrite.writeFeatureModel(SavePath.toStdString());
}



///Spawn File Selection popup to get a Feature Model
void FeatureModelEditor::findModel() {
   QString const Path = QFileDialog::getOpenFileName(this,tr("Open Model"),"/home",tr("XML files (*.xml)"));
   Ui->ModelFile->setText(Path);
}


/// Loead the source files of the Feature to be selectable by the user and set the Feature as CurrentFeature.
/// \param Feature
void FeatureModelEditor::inspectFeatureSources(vara::feature::Feature *Feature) {
    CurrentFeature = Feature;
    if(Repository.isEmpty()){
    Repository = QFileDialog::getExistingDirectory();
    }
    Ui->sources->clear();
    for(const auto& Source : Feature->getLocations()){
      Ui->sources->addItem(QString::fromStdString(Source.getPath().string()));
    }
    Ui->sourcesLable->setText( QString::fromStdString("Sources for: "+Feature->getName().str()));
    if(Ui->sources->count() == 1){
      loadSource(Ui->sources->itemText(0));
    } else {
      Ui->sources->setPlaceholderText("Select File");
    }
}


/// Load the selected file into the textedit and mark the sources of the selected feature
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
        markLocation(Fmt, Cursor, Location);
      }
  }

}
/// Mark the given SourceRange with the given Format
/// \param Fmt Format to mark with
/// \param Cursor  Cursor
/// \param Location Location to mark
void FeatureModelEditor::markLocation(
    const QTextCharFormat &Fmt, QTextCursor &Cursor,
    vara::feature::FeatureSourceRange &Location)
    const {
  Cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  Cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor,
                      Location.getStart()->getLineNumber() - 1);
  Cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor,
                      Location.getStart()->getColumnOffset()-1);
  Cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,
                      Location.getEnd()->getLineNumber() -
                          Location.getStart()->getLineNumber());
  Cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
  Cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor,
                      Location.getEnd()->getColumnOffset());
  Cursor.setCharFormat(Fmt);
}

/// Create the Context menu for inspecting sources in the tree view
/// \param Pos Position of the cursor used to find the clicked item
void FeatureModelEditor::createTreeContextMenu(const QPoint &Pos) {
  auto Index = TreeView->indexAt(Pos);
  if(Index.isValid()){
      FeatureTreeItem* Item = static_cast<FeatureTreeItem*>(Index.internalPointer());
      Item->contextMenu(TreeView->mapToGlobal(Pos));
  }

}

///Load a sourcefile to then add a location from it
void FeatureModelEditor::addSourceFile(){
  if(!Repository.isEmpty()) {
      QString const Path = QFileDialog::getOpenFileName(
          this, tr("Select Source File"), Repository,
          tr("C Files (*.c *c++ *.h)"));
      Ui->sources->addItem(Path.sliced(Repository.length()));
  }
}
/// Add the user selected Part of the textedit as a source for the active Feature
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
  auto LocationTransAction = Transaction::openTransaction(*FeatureModel);
  LocationTransAction.addLocation(CurrentFeature,Range);
  LocationTransAction.commit();
  loadSource(Ui->sources->currentText());
}
