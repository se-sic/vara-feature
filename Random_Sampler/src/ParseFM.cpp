#include "Plotter.h"
#include "ParseFM.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

using std::string;


namespace bdd::sample {

    std::unique_ptr<vara::feature::FeatureModel> ParseXML(std::string &FilePath) { //NOLINT
        // Read the file content
        std::ifstream FileIn(FilePath);
        if (!FileIn) {
            throw std::runtime_error("Could not open file: " + FilePath);
        }
        std::ostringstream Oss;
        Oss << FileIn.rdbuf();
        std::string XMLContent = Oss.str();

        // Parse the content
        vara::feature::FeatureModelXmlParser Parser(XMLContent);

        // Verify if the feature model is valid
        auto Verify = Parser.verifyFeatureModel();
        if(!Verify) {
            throw std::runtime_error("Error parsing XML: verification failed");
        }   

        // Build the feature model object
        auto Fm = Parser.buildFeatureModel();
        if(!Fm) {
            throw std::runtime_error("Error building Feature Model: ");
        }

        return Fm;
    }
} // namespace bdd::sample