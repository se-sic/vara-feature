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

unique_ptr<FeatureModel> parseFromFile(const string &filename) {
    ifstream file(filename);
    if (!file) {
        cerr << "Error parsing feature model from file: " << filename << "\n";
        return nullptr;
    }

    stringstream buffer;
    buffer << file.rdbuf();

    FeatureModelXmlParser parser(buffer.str());

    if(!parser.verifyFeatureModel()) {
        cerr << "Error verifying feature model.\n";
        return nullptr;
    } else {
        return parser.buildFeatureModel();
    }
}
