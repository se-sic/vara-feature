//
// Created by simon on 04.11.22.
//

#include "FeatureModelEditor.h"
#include "graph/FeatureModelGraph.h"
#include "vara/Feature/FeatureModel.h"

#include <QApplication>
#include <QMainWindow>
#include <QTime>

int main(int argc, char **argv)
{
  QApplication App(argc, argv);
  FeatureModelEditor W;
  W.show();
  return App.exec();
}