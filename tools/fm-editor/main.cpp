//
// Created by simon on 04.11.22.
//

#include "FeatureModelGraph.h"
#include "vara/Feature/FeatureModel.h"

#include <QApplication>
#include <QTime>
#include <QMainWindow>

int main(int argc, char **argv)
{
  QApplication App(argc, argv);
  auto Model =  vara::feature::loadFeatureModel("test_children.xml");
  auto *Widget = new FeatureModelGraph(Model.get());

  QMainWindow MainWindow;
  MainWindow.setCentralWidget(Widget);

  MainWindow.show();
  return App.exec();
}