#ifndef FD_PARSER
#define FD_PARSER

#include "vara/Feature/FeatureModelParser.h"
#include <fstream>
#include <sstream>
#include <iostream>

using std::string;
using std::unique_ptr;
using std::ifstream;
using std::cerr;
using std::stringstream;
using namespace vara::feature;

unique_ptr<FeatureModel> parseFromFile(const string &filename);

#endif // FD_PARSER

