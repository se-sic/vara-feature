//
// Created by simon on 29.11.22.
//

#ifndef VARA_FEATURE_FEATUREADDDIALOG_H
#define VARA_FEATURE_FEATUREADDDIALOG_H
#include "graph/FeatureModelGraph.h"
#include "ui_FeatureAddDialog.h"
#include <QDialog>
#include <QInputDialog>

class FeatureAddDialog : public QDialog, public Ui::Add {
  Q_OBJECT
public:
  FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent);
  QString getName();
  QString getParent();
  QString getOutpuString();
  std::unique_ptr<vara::feature::Feature> getFeature();
  vara::feature::Feature::FeatureKind getFeatureKind();
  bool isOptional();
public slots:
  void featureType(int index);
private:

  QStringList NodeNames;


  vara::feature::StepFunction::StepOperation getStepOperation();
};

#endif // VARA_FEATURE_FEATUREADDDIALOG_H
