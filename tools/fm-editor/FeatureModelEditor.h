
#ifndef VARA_FEATURE_FEATUREMODELEDITOR_H
#define VARA_FEATURE_FEATUREMODELEDITOR_H

#include "vara/Feature/Feature.h"
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
public slots:
  void loadFeature(vara::feature::Feature *Feature);
  void loadGraph();
};

#endif // VARA_FEATURE_FEATUREMODELEDITOR_H
