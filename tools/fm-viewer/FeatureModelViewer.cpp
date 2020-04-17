#include "vara/Feature/Feature.h"

int main(/* int argc, char *argv[] */) {

  vara::feature::Feature f;
  f.addStuff(1);
  f.addStuff(2);
  f.addStuff(3);
  f.doStuff();

  return 0;
}
