#include "vara/Feature/FeatureModelParser.h"
#include "vara/Feature/FeatureModel.h"
#include "../../BDD/include/BDDFactory.h"
#include "BDDSampler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

int main(int argc, char* argv[]) {}
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <feature_model.xml>\n";
//         return 1;
//     }

//     std::string filePath = argv[1];

//     std::unique_ptr<vara::feature::FeatureModel> fd = [&]() {
//         // 1. Datei einlesen
//         std::ifstream in(filePath);
//         if (!in) {
//             throw std::runtime_error("Could not open file: " + filePath);
//         }
        
//         // 2. Inhalt in String umwandeln
//         std::ostringstream oss;
//         oss << in.rdbuf();
//         std::string xmlContent = oss.str();
//         // 3. XML parsen
//         vara::feature::FeatureModelXmlParser parser(xmlContent);

//         // 4. Validierung
//         auto verify = parser.verifyFeatureModel();
//         if(!verify) {
//             throw std::runtime_error("Error parsing XML");
//         }

//         // 5. Modell erstellen
//         auto fm = parser.buildFeatureModel();
//         if(!fm) {
//             throw std::runtime_error("Error building Feature Model");
//         }

//         // 6. Zusatzverarbeitung (optional)
//         (void)vara::feature::FeatureModelXmlParser::detectXMLAlternatives(*fm);

//         return fm;
//     }();

//     // BDD-Verarbeitung
//     std::unordered_map<std::string, oxidd::bdd_function> featureVars;
//     std::vector<std::pair<oxidd::bdd_function, std::string>> varList;

//     oxidd::capi::BDDFactory factory;
//     std::cout << "Fight2" << std::endl;
//     oxidd::capi::oxidd_bdd_t finalBDD = factory.modelToBdd(*fd); 
    
//     // Konfiguration generieren
//     auto sample = oxidd::capi::generateConfiguration(
//         finalBDD,
//         factory
//     );

//     // Ergebnisse ausgeben
//     std::cout << "Sampled Configuration:\n";
//     for (const auto& [feature, enabled] : sample) {
//         std::cout << "  " << feature 
//                  << ": " << (enabled ? "true" : "false") << '\n';
//     }

//     return 0;
// }