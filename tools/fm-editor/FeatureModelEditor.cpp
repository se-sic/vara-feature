#include "FeatureModelEditor.h"
#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
#include "ui_FeatureModelEditor.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"
#include "vara/Feature/FeatureModelWriter.h"

#include <QDir>
#include <QFileDialog>
#include <QTextStream>

using Transaction = vara::feature::FeatureModelTransaction<
    vara::feature::detail::ModifyTransactionMode>;
using vara::feature::Feature;

FeatureModelEditor::FeatureModelEditor(QWidget *Parent)
    : QMainWindow(Parent), Ui(new Ui::FeatureModelEditor) {

  Ui->setupUi(this);
  Ui->textEdit->setReadOnly(true);
  Highlighter = std::make_unique<QSourceHighlite::QSourceHighliter>(
      Ui->textEdit->document());
  Highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeCpp);
  QObject::connect(Ui->loadModel, &QPushButton::pressed, this,
                   &FeatureModelEditor::loadGraph);
  QObject::connect(Ui->actionAddFeature, &QAction::triggered, this,
                   &FeatureModelEditor::featureAddDialog);
  connect(Ui->actionSave, &QAction::triggered, this, &FeatureModelEditor::save);
  connect(Ui->addSource, &QPushButton::pressed, this,
          &FeatureModelEditor::addSource);
  connect(Ui->addSourceFile, &QPushButton::pressed, this,
          &FeatureModelEditor::addSourceFile);
}

/// Display the information of a Feature
void FeatureModelEditor::loadFeature(const vara::feature::Feature *Feature) {
  Ui->featureInfo->setText(QString::fromStdString(Feature->toString()));
}

/// Get a Feature from an Index of the TreeView and display its information.
void FeatureModelEditor::loadFeatureFromIndex(const QModelIndex &Index) {
  if (Index.isValid()) {
    auto *Item = static_cast<FeatureTreeItem *>(Index.internalPointer())
                     ->child(Index.row());
    if (Item->getKind() ==
        vara::feature::FeatureTreeNode::NodeKind::NK_FEATURE) {
      loadFeature(dynamic_cast<FeatureTreeItemFeature *>(Item)->getFeature());
    }
  }
}

/// Clear all Fields that should be emptied when loading a new Model
void FeatureModelEditor::clean() {
  SavePath.clear();
  Repository.clear();
  Ui->tabWidget->clear();
  Ui->sources->clear();
  Ui->textEdit->clear();
  Ui->sourcesLable->setText("Sources for: ");
  Ui->featureInfo->clear();
}

/// Load the Feature Model at the Path in ModelFile field and build the Views
void FeatureModelEditor::loadGraph() {
  clean();
  ModelPath = Ui->ModelFile->text();
  FeatureModel = vara::feature::loadFeatureModel(ModelPath.toStdString());
  if (!FeatureModel) {
    QString const Path = QFileDialog::getOpenFileName(
        this, tr("Open Model"), "/home", tr("XML files (*.xml)"));
    if (Path.isEmpty()) {
      return;
    }
    Ui->ModelFile->setText(Path);
    FeatureModel = vara::feature::loadFeatureModel(Path.toStdString());
  }

  // create Graph view
  buildGraph();

  // create Tree View
  buildTree();

  Ui->tabWidget->addTab(Graph.get(), "GraphView");
  Ui->tabWidget->addTab(TreeView.get(), "TreeView");
  connect(Ui->sources, &QComboBox::currentTextChanged, this,
          &FeatureModelEditor::loadSource);
  Ui->actionSave->setEnabled(true);
  Ui->actionAddFeature->setEnabled(true);
}

/// Build the Treeview
void FeatureModelEditor::buildTree() {
  if (!TreeView) {
    TreeView = std::make_unique<QTreeView>(this);
  }
  TreeModel = std::make_unique<FeatureTreeViewModel>(FeatureModel.get(),
                                                     TreeView.get());
  for (auto &Item : *TreeModel->getItems()) {
    connect(Item.get(), &FeatureTreeItem::inspectSource, this,
            &FeatureModelEditor::inspectFeatureSources);
    connect(Item.get(), &FeatureTreeItem::addChildFeature, this,
            &FeatureModelEditor::featureAddDialogChild);
    connect(Item.get(), &FeatureTreeItem::removeFeature, this,
            &FeatureModelEditor::removeFeature);
  }
  connect(TreeView.get(), &QTreeView::pressed, this,
          &FeatureModelEditor::loadFeatureFromIndex);
  TreeView->setModel(TreeModel.get());
  TreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(TreeView.get(), SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(createTreeContextMenu(QPoint)));
}

/// Build the graph view
void FeatureModelEditor::buildGraph() {
  Graph = std::make_unique<FeatureModelGraph>(FeatureModel.get());
  for (auto &Node : *Graph->getNodes()) {
    QObject::connect(Node.get(), &FeatureNode::clicked, this,
                     &FeatureModelEditor::loadFeature);
    QObject::connect(Node.get(), &FeatureNode::inspectSource, this,
                     &FeatureModelEditor::inspectFeatureSources);
  }
}

void FeatureModelEditor::featureAddDialog() { featureAddDialogChild(nullptr); }

/// Spawn a Dialog to select data to add a Feature
void FeatureModelEditor::featureAddDialogChild(Feature *ParentFeature) {
  FeatureAddDialog AddDialog(Graph.get(), this, ParentFeature);
  if (AddDialog.exec() == QDialog::Accepted) {
    Feature *Parent =
        FeatureModel->getFeature(AddDialog.getParent().toStdString());
    auto NewFeature = AddDialog.getFeature();

    auto *NewNode = Graph->addNode(NewFeature.get(),
                                   Graph->getNode(Parent->getName().str()));
    auto *NewTreeItem =
        TreeModel->addFeature(NewFeature.get(), Parent->getName().str());
    connect(NewTreeItem, &FeatureTreeItem::inspectSource, this,
            &FeatureModelEditor::inspectFeatureSources);
    connect(NewNode, &FeatureNode::clicked, this,
            &FeatureModelEditor::loadFeature);
    connect(NewNode, &FeatureNode::inspectSource, this,
            &FeatureModelEditor::inspectFeatureSources);
    auto Transaction = vara::feature::FeatureModelTransaction<
        vara::feature::detail::ModifyTransactionMode>::
        openTransaction(*FeatureModel);
    auto PotentialRelations =
        Parent->getChildren<vara::feature::Relationship>(1);
    if (!PotentialRelations.empty()) {
      Transaction.addFeature(std::move(NewFeature),
                             *PotentialRelations.begin());
    } else {
      Transaction.addFeature(std::move(NewFeature), Parent);
    }
    Transaction.commit();
  }
}

void FeatureModelEditor::removeFeature(bool Recursive, Feature *Feature) {
  TreeModel->deleteFeatureItem(Recursive, Feature);
  Graph->deleteNode(Recursive, Feature);
  Ui->featureInfo->clear();
  vara::feature::removeFeature(*FeatureModel, Feature, Recursive);
}

/// Save the current State of the Feature Model
void FeatureModelEditor::save() {
  SavePath = QFileDialog::getSaveFileName(this, tr("Save File"), ModelPath,
                                          tr("XML files (*.xml)"));
  vara::feature::FeatureModelXmlWriter FMWrite{*FeatureModel};
  FMWrite.writeFeatureModel(SavePath.toStdString());
}

/// Load the source files of the Feature to be selectable by the user and set
/// the Feature as CurrentFeature.
///
/// \param Feature Selected Feature
void FeatureModelEditor::inspectFeatureSources(
    vara::feature::Feature *Feature) {
  CurrentFeature = Feature;
  if (Repository.isEmpty()) {
    Repository = QFileDialog::getExistingDirectory();
  }
  Ui->sources->clear();
  for (const auto &Source : Feature->getLocations()) {
    Ui->sources->addItem(QString::fromStdString(Source.getPath().string()));
  }
  Ui->sourcesLable->setText(
      QString::fromStdString("Sources for: " + Feature->getName().str()));
  if (Ui->sources->count() == 1) {
    loadSource(Ui->sources->itemText(0));
  }
  else {
    Ui->sources->setPlaceholderText("Select File");
  }
}

/// Create the Context menu for inspecting sources in the tree view
///
/// \param Pos Position of the cursor used to find the clicked item
void FeatureModelEditor::createTreeContextMenu(const QPoint &Pos) {
  auto Index = TreeView->indexAt(Pos);
  if (Index.isValid()) {
    FeatureTreeItem *Item =
        static_cast<FeatureTreeItem *>(Index.internalPointer())
            ->child(Index.row());
    Item->contextMenu(TreeView->mapToGlobal(Pos));
  }
}

/// Load the selected file into the textedit and mark the sources of the
/// selected feature
///
/// \param RelativePath  path to the source file relative to the repository path
void FeatureModelEditor::loadSource(const QString &RelativePath) {
  Ui->textEdit->clear();
  auto SourcePath = Repository + "/" + RelativePath;
  QFile File(SourcePath);
  if (File.exists()) {
    File.open(QFile::ReadOnly | QFile::Text);
    QTextStream ReadFile(&File);
    Ui->textEdit->setText(ReadFile.readAll());
    std::vector<vara::feature::FeatureSourceRange> Locations{};
    std::copy_if(
        CurrentFeature->getLocationsBegin(), CurrentFeature->getLocationsEnd(),
        std::back_inserter(Locations), [&RelativePath](auto const &Loc) {
          return RelativePath.toStdString() == Loc.getPath();
        });
    for (auto &Location : Locations) {
      markLocation(Location);
    }
  }
}
void setCursorLineAndColumn(QTextCursor &Cursor, int Line, int Col,
                            QTextCursor::MoveMode Mode) {
  QTextBlock const B = Cursor.document()->findBlockByLineNumber(Line);
  Cursor.setPosition(B.position() + Col, Mode);
}

/// Mark the given SourceRange with the given Format
///
/// \param Fmt Format to mark with
/// \param Cursor  Cursor
/// \param Location Location to mark
void FeatureModelEditor::markLocation(
    vara::feature::FeatureSourceRange &Location) const {
  QTextCharFormat Fmt;
  Fmt.setBackground(Qt::darkYellow);
  QTextCursor Cursor(Ui->textEdit->document());
  Cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
  setCursorLineAndColumn(Cursor, int(Location.getStart()->getLineNumber()) - 1,
                         int(Location.getStart()->getColumnOffset()) - 1,
                         QTextCursor::MoveAnchor);
  setCursorLineAndColumn(Cursor, int(Location.getEnd()->getLineNumber()) - 1,
                         int(Location.getEnd()->getColumnOffset()) - 1,
                         QTextCursor::KeepAnchor);
  Cursor.setCharFormat(Fmt);
}

/// Load a sourcefile to then add a location from it
void FeatureModelEditor::addSourceFile() {
  if (!Repository.isEmpty()) {
    QString const Path =
        QFileDialog::getOpenFileName(this, tr("Select Source File"), Repository,
                                     tr("C Files (*.c *c++ *.h)"));
    Ui->sources->addItem(Path.mid(Repository.length()));
    Ui->sources->setCurrentIndex(Ui->sources->count()-1);
  }
}

/// Add the user selected Part of the textedit as a source for the active
/// Feature
void FeatureModelEditor::addSource() {
  auto *TextEdit = Ui->textEdit;
  auto Cursor = TextEdit->textCursor();
  const int Start = Cursor.selectionStart();
  const int End = Cursor.selectionEnd();
  Cursor.movePosition(QTextCursor::MoveOperation::StartOfLine);
  const int LineStart = Cursor.position();
  int Lines = 1;
  auto Block = Cursor.block();
  while (Cursor.position() > Block.position()) {
    Cursor.movePosition(QTextCursor::MoveOperation::Up);
    Lines++;
  }
  Block = Block.previous();
  while (Block.isValid()) {
    Lines += Block.lineCount();
    Block = Block.previous();
  }
  auto Range = vara::feature::FeatureSourceRange(
      Ui->sources->currentText().toStdString(),
      vara::feature::FeatureSourceRange::FeatureSourceLocation(
          Lines, Start - LineStart + 1),
      vara::feature::FeatureSourceRange::FeatureSourceLocation(
          Lines, End - LineStart));
  auto LocationTransAction = Transaction::openTransaction(*FeatureModel);
  LocationTransAction.addLocation(CurrentFeature, Range);
  LocationTransAction.commit();
  markLocation(Range);
}
