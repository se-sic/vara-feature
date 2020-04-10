#include "vara/Feature/Feature.h"

#include <iostream>

namespace vara::feature {

void Feature::doStuff() {
  std::cout << "Data size: " << Data.size() << '\n';
  std::cout << "Doing stuff" << '\n';
}

void Feature::addStuff(int I) { Data.emplace_back(I); }

} // namespace vara::feature
