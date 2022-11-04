//
// Created by simon on 04.11.22.
//

#include "FeatureModelGraph.h"

#include <QApplication>
#include <QTime>
#include <QMainWindow>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  GraphWidget *widget = new FeatureModelGraph();

  QMainWindow mainWindow;
  mainWindow.setCentralWidget(widget);

  mainWindow.show();
  return app.exec();
}