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
  string getParent();
  vara::feature::Feature::FeatureKind getFeatureKind();
public slots:
  void featureType(int index);
private:

  QStringList NodeNames;

};

#endif // VARA_FEATURE_FEATUREADDDIALOG_H
