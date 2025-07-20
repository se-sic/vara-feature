// // #include "vara/Solver/Z3Solver.h"
// // #include "FeatureDiagram.hpp"
// // #include "vara/Feature/FeatureModel.h"
// // #include "Feature.h"
// // #include "Z3Helper.hpp"

// // #include <z3++.h>
// // #include <unordered_map>
// // #include <string>
// // #include <iostream>

// // using vara::solver::Z3Solver;

// std::unordered_map<Feature*, int>
// count_valid_configs_from_featureModel(const std::string& xmlPath, FeatureModel* model) {
//     std::unordered_map<Feature*, int> featureCounts;
//     std::unordered_map<std::string, Feature*> nameToFeature;

//     z3::context ctx;
//     z3::solver solver(ctx);

//     Z3_solver raw_solver = solver;
//     addXmlConstraintsToSolver(raw_solver, xmlPath);

//     std::unordered_map<std::string, z3::expr> allVars;

//     while (solver.check() == z3::sat) {
//         z3::model m = solver.get_model();
//         z3::expr_vector blocking(ctx);

//         for (const auto& [name, expr] : allVars) {
//             z3::expr val = m.eval(expr, true);
//             bool isTrue = val.is_bool() && val.bool_value() == Z3_L_TRUE;
            
//             if (isTrue && nameToFeature.count(name)) {
//                 Feature* f = nameToFeature.at(name);
//                 featureCounts[f]++;
//             }

//             blocking.push_back(isTrue ? !expr : expr);
//         }

//         solver.add(z3::mk_or(blocking));
//     }

// //     return featureCounts;
// // }

// // std::unordered_map<Feature*, int>
// // count_valid_configs_from_xml(const std::string& xmlPath) {
// //     FeatureModel* model = FeatureDiagram::parseFeatureModelFromXML(xmlPath);
// //     return count_valid_configs_from_featureModel(xmlPath, model);
// // }

// int main(int argc, char** argv) {
//     if (argc != 2) {
//         std::cerr << "Usage: " << argv[0] << " <path_to_xml_file>" << std::endl;
//         return 1;
//     }

//     auto featureCounts = count_valid_configs_from_xml(argv[1]);

// //     int total = 0;
// //     for (const auto& [f, n] : featureCounts)
// //         total += n;

//     std::cout << "\nTotal feature activations across all valid configurations: " 
//               << total << "\n\n";
//     for (const auto& [f, n] : featureCounts) {
//         double percent = (100.0 * n) / total;
//         std::cout << "Feature \"" << std::string(f->getName()) << "\": "
//                   << n << "x (" << percent << "%)" << std::endl;
//     }

// //     return 0;
// // }