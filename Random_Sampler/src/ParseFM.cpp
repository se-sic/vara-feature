#include "ParseFM.h"

#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelParser.h"

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace bdd::sample {

    std::unique_ptr<vara::feature::FeatureModel> ParseXML(const std::string &FilePath) { //NOLINT
        std::ifstream FileIn(FilePath);
        if (!FileIn) {
            std::cerr << "Error: Could not open file " << FilePath << "\n";
            return nullptr;
        }

        std::ostringstream Oss;
        Oss << FileIn.rdbuf();
        std::string XMLContent = Oss.str();

        vara::feature::FeatureModelXmlParser Parser(XMLContent);

        auto Verify = Parser.verifyFeatureModel();
        if(!Verify) {
            std::cerr << "Error: XML verification failed for file " << FilePath << "\n";
            return nullptr;
        }   

        auto Fm = Parser.buildFeatureModel();
        if(!Fm) {
            std::cerr << "Error: Failed to build feature model from XML for file " << FilePath << "\n";
            return nullptr;
        }

        return Fm;
    }
} // namespace bdd::sample