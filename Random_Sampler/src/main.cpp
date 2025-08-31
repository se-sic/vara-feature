<<<<<<< HEAD
=======
// //#include "ConfigCounter.hpp"
// #include "BDDSampler.h"
// #include "Z3Helper.hpp"
// #include "FeatureDiagram.hpp"
// #include "GraphViz.h"
// #include "Sampler.hpp"
// #include "NodeType.hpp"
#include "vara/Feature/FeatureModelParser.h"
>>>>>>> 7763f2c0 (rework of BDDFAactory, BDDFeats, and probs)
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
<<<<<<< HEAD

int main(int argc, char* argv[]) {}
//     if (argc < 2) {
//         std::cerr << "Usage: " << argv[0] << " <feature_model.xml>\n";
//         return 1;
//     }
=======
#include <cstdlib>
extern "C" {
#include <oxidd/capi.h>
}

// using oxidd::bdd_manager;
// using oxidd::bdd_function;
// using oxidd::capi::oxidd_bdd_manager_t;
// using oxidd::capi::oxidd_bdd_t;
using std::string;
using std::vector;
using std::pair;
>>>>>>> 7763f2c0 (rework of BDDFAactory, BDDFeats, and probs)

//     std::string filePath = argv[1];

<<<<<<< HEAD
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
=======
 int main(int argc, char* argv[]) { }
>>>>>>> 7763f2c0 (rework of BDDFAactory, BDDFeats, and probs)

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

<<<<<<< HEAD
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
=======
    std::unique_ptr<vara::feature::FeatureModel> fd = [&]() {
        //reads the file in
        std::ifstream in(filePath);
        if (!in) {
            throw std::runtime_error("Could not open file: " + filePath);
        }
        std::ostringstream oss;
        oss << in.rdbuf();
        std::string xmlContent = oss.str();

        //parse the file
        vara::feature::FeatureModelXmlParser parser(xmlContent);

        auto verify = parser.verifyFeatureModel();
        if(!verify) {
            throw std::runtime_error("Error parsing XML: " + verify);
        }   

        auto fm = parser.buildFeatureModel();
        if(!fm) {
            throw std::runtime_error("Error building Feature Model: ");
        }

        (void)vara::feature::FeatureModelXmlParser::detectXMLAlternatives(*fm);

        return fm;
    }();

    std::cout << "Feature Model loaded successfully from: " << filePath << std::endl;

    oxidd::capi::BDDFactory factory;
    oxidd_bdd_t finalBDD = factory.modelToBdd(*fd);
    oxidd_bdd_manager_t manager = oxidd_bdd_containing_manager(finalBDD);
    std::cout << "BDD constructed successfully." << std::endl;

    for (const auto& [level, feat] : factory.varMap) {   
        std::cout << "Feature: " << feat.name
                  << ", Level: " << level 
                  << ", Probability: " 
                  << (feature->probability.has_value() ? std::to_string(feature->probability.value()) : "N/A") 
                  << std::endl;
    }

    

//     oxidd::capi::BDDFactory factory;
//     oxidd_bdd_t finalBDD = factory.modelToBdd(*fd); 
//     auto sample =  oxidd::capi::generateConfiguration(
//                 finalBDD,
//                 factory
//             );

//     std::cout << "Sampled Configuration:\n";
//     for (const auto& [level, enabled] : sample) {
//         std::cout << "  level " << level
//                 << " -> " << (enabled ? "true" : "false") << '\n';
//     }


//     return 0;

    // auto children = fd->getRoot()->getChildren<Feature>();
    // for(Feature *c : children) {
    //     std::cout << "Child Feature: " << c->getName().str() << std::endl;
    //     if(c->isOptional()) {
    //         std::cout << "  - Optional Feature" << std::endl;
    //     } else {
    //         std::cout << "  - Mandatory Feature" << std::endl;
    //     }
    //     checkType(*fd, c);
    //     auto child = c->getChildren<Feature>();
    //     if (child.empty()) {
    //         std::cout << "  - Leaf Feature" << std::endl;
    //     } else {
    //         std::cout << "  - Non-Leaf Feature with children: ";
    //         for(Feature *cici : child) {
    //             std::cout << cici->getName().str() << " ";
    //             checkType(*fd, cici);
    //         }
    //     }
        // switch(checkType(*fd, c)) {
        //     case NodeType::AND:
        //         std::cout << "  - AND Node" << std::endl;
        //         break;
        //     case NodeType::OR:
        //         std::cout << "  - OR Node" << std::endl;
        //         break;
        //     case NodeType::XOR:
        //         std::cout << "  - XOR Node" << std::endl;
        //         break;
        //     case NodeType::LEAF:
        //         std::cout << "  - Leaf Node" << std::endl;
        //         if(c->getChildren<Feature>().empty()) {
        //             std::cout << "    - No children, this is a leaf node." << std::endl;
        //         } else {
        //             std::cout << "    - Has children, not a true leaf." << std::endl;
        //         }
        //         break;
        //     default:
        //         std::cout << "  - Unknown Node Type" << std::endl;
        // }
>>>>>>> 7763f2c0 (rework of BDDFAactory, BDDFeats, and probs)
    
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

<<<<<<< HEAD
//     return 0;
// }
=======

    // for (const auto &entry : sample) {
    //     std::cout << entry.first->getName().str() << ": " << (entry.second ? "true" : "false") << std::endl;
    // }

    // return 0;




    // oxidd::bdd_manager mgr(32, 320, 1);

    // oxidd::bdd_function a = mgr.new_var();
    // oxidd::bdd_function b = mgr.new_var();
    // oxidd::bdd_function c =  mgr.new_var();
    // oxidd::bdd_function f = a & b | ~a & c | b & ~c;

    // const oxidd_bdd_manager_t* c_mgr = reinterpret_cast<const oxidd_bdd_manager_t*>(&mgr);

    // const oxidd_bdd_t* c_a = reinterpret_cast<const oxidd_bdd_t*>(&a);
    // const oxidd_bdd_t* c_b = reinterpret_cast<const oxidd_bdd_t*>(&b);
    // const oxidd_bdd_t* c_c = reinterpret_cast<const oxidd_bdd_t*>(&c);
    // const oxidd_bdd_t* c_f = reinterpret_cast<const oxidd_bdd_t*>(&f);

    // const oxidd_bdd_t functions[] = { *c_f };
    // const char* function_names[] = { "Function" };

    // const oxidd_bdd_t vars[] = { *c_a, *c_b, *c_c };
    // const char* var_names[] = { "a", "b", "c"};

    // bool check = oxidd_bdd_manager_dump_all_dot_file(
    //     *c_mgr,
    //     "../results/bdd.dot",
    //     functions,
    //     function_names,
    //     1,
    //     vars,
    //     var_names,
    //     3
    // );

}
>>>>>>> 7763f2c0 (rework of BDDFAactory, BDDFeats, and probs)
