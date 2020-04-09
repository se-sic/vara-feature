#include "vara/Feature/Feature.h"

#include <iostream>

namespace vara::feature {

void Feature::doStuff() {
  std::cout << "Data size: " << data.size() << '\n';
  std::cout << "Doing stuff" << '\n';
}

void Feature::addStuff(int i) { data.emplace_back(i); }

} // namespace vara::feature
