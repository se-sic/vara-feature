
#ifndef VARA_FEATURE_FEATUREMODELEDITOR_H
#define VARA_FEATURE_FEATUREMODELEDITOR_H

#include "graph/FeatureModelGraph.h"
#include "tree/FeatureTreeViewModel.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"
#include <QDialog>
#include <QMainWindow>
#include <QTextCharFormat>
#include <QTreeView>
QT_BEGIN_NAMESPACE
namespace Ui {
class FeatureModelEditor;
} // namespace Ui
QT_END_NAMESPACE

class FeatureModelEditor : public QMainWindow {
  Q_OBJECT
public:
  explicit FeatureModelEditor(QWidget *Parent = nullptr);
  ~FeatureModelEditor() override = default;

private:
  Ui::FeatureModelEditor *Ui;
  std::unique_ptr<FeatureModelGraph> Graph{};
  QTreeView *TreeView;
  std::unique_ptr<FeatureTreeViewModel> TreeModel{};
  std::unique_ptr<vara::feature::FeatureModel> FeatureModel{};
  QString Repository{};
  vara::feature::Feature *CurrentFeature;
  QString SavePath{};
  QString ModelPath{};
public slots:
  void addSource();
  void loadFeature(const vara::feature::Feature *Feature);
  void inspectFeatureSources(vara::feature::Feature *Feature);
  void loadGraph();
  void featureAddDialogChild(vara::feature::Feature * = nullptr);
  // void addNode(const QString& Name, FeatureNode *Parent);
  void loadSource(const QString &RelativePath);
  void createTreeContextMenu(const QPoint &Pos);
  void addSourceFile();
  void loadFeatureFromIndex(const QModelIndex &Index);
  void save();
  void featureAddDialog();
  void removeFeature(bool Recursive, vara::feature::Feature *Feature);

private:
  void clean();
  void buildGraph();
  void buildTree();
  void markLocation(vara::feature::FeatureSourceRange &Location) const;
};

#endif // VARA_FEATURE_FEATUREMODELEDITOR_H
