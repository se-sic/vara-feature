#include "../include/ConfigCounter.hpp"
#include "../include/Z3Helper.hpp"
#include "../include/FeatureDiagram.hpp"
#include "../include/GraphViz.hpp"
#include "../include/Sampler.hpp"
#include <iostream>
#include <unordered_map>
#include <random>
#include <string>
#include <vector>
#include <cstdlib>


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_feature_model.xml>" << std::endl;
        return 1;
    }

    std::string xmlPath = argv[1];

    unique_ptr<FeatureModel> fd = parseFromFile(xmlPath);
    std::unordered_map<Feature*, int> cc = count_valid_configs_from_featureModel(xmlPath, fd.get());

    oxidd::bdd_manager mgr;
    unordered_map<std::string, oxidd::bdd_function> featureVars;
    vector<pair<oxidd::bdd_function, std::string>> varList;
    
    std::random_device rd;
    unordered_map<Feature * , bool> sample = sampleRandomly(*fd, cc, rd);


    for (const auto &entry : sample) {
        std::cout << entry.first->getName().str() << ": " << (entry.second ? "true" : "false") << std::endl;
    }

    return 0;
}