#include "FeatureModelEditor.h"

#include <QApplication>

int main(int argc, char **argv) {
  const QApplication App(argc, argv);
  FeatureModelEditor W;
  W.show();
  return QApplication::exec();
}
