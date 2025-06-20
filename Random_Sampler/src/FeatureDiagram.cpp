#include "FeatureDiagram.hpp"

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
