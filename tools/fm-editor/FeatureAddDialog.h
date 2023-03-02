#ifndef VARA_FEATURE_FEATUREADDDIALOG_H
#define VARA_FEATURE_FEATUREADDDIALOG_H

#include "graph/FeatureModelGraph.h"
#include "ui_FeatureAddDialog.h"

#include <QDialog>
#include <QInputDialog>

class FeatureAddDialog : public QDialog, public Ui::Add {
  Q_OBJECT
public:
  FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent,
                   vara::feature::Feature *ParentFeature = nullptr);
  QString getName() const;
  QString getParent() const;
  QString getOutputString() const;
  std::unique_ptr<vara::feature::Feature> getFeature();
  vara::feature::Feature::FeatureKind getFeatureKind();
  bool isOptional() const;

public slots:
  void featureType(int index);

private:
  QStringList NodeNames;

  std::unique_ptr<vara::feature::Feature> getNumericFeature() const;
};

#endif // VARA_FEATURE_FEATUREADDDIALOG_H
