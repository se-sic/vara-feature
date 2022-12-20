
#ifndef VARA_FEATURE_FEATUREMODELEDITOR_H
#define VARA_FEATURE_FEATUREMODELEDITOR_H

#include "graph/FeatureModelGraph.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelTransaction.h"
#include <QDialog>
#include <QMainWindow>
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
  FeatureModelGraph * Graph{};
  std::unique_ptr<vara::feature::FeatureModel> Model {};
  std::unique_ptr<vara::feature::FeatureModelTransaction<vara::feature::detail::ModifyTransactionMode>> Modification {};
public slots:
  void loadFeature(vara::feature::Feature *Feature);
  void loadGraph();
  void featureAddDialog();
  //void addFeature(const QString& Name, FeatureNode *Parent);

};

#endif // VARA_FEATURE_FEATUREMODELEDITOR_H
