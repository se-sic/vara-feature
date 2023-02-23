#include "FeatureModelEditor.h"

#include <QApplication>

int main(int argc, char **argv) {
  QApplication const App(argc, argv);
  FeatureModelEditor W;
  W.show();
  return App.exec();
}