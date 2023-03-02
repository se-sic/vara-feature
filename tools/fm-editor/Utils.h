#ifndef VARA_FEATURE_UTILS_H
#define VARA_FEATURE_UTILS_H

#include <QMenu>

template <class T>
struct ActionBuilder {
  ActionBuilder(QMenu *Menu, T *Receiver) : Menu(Menu), Receiver(Receiver) {}
  template <class Func1, class... Func>
  void addActions(std::pair<QString, Func1> Action,
                  std::pair<QString, Func>... Actions) {
    Menu->addAction(Action.first,Receiver,Action.second);
    addActions(Actions...);
  }
  void addActions() {}

private:
  QMenu *Menu;
  T *Receiver;
};

template <class T, class... Func>
std::unique_ptr<QMenu> buildMenu(T *Receiver, std::pair<QString, Func>... Actions) {
  auto Menu = std::make_unique<QMenu>();
  ActionBuilder Builder(Menu.get(), Receiver);
  Builder.addActions(Actions...);
  return Menu;
}

#endif // VARA_FEATURE_UTILS_H