// #include "vara/Solver/Z3Solver.h"
// #include "FeatureDiagram.hpp"
// #include "vara/Feature/FeatureModel.h"
// #include "Feature.h"
// #include "Z3Helper.hpp"

// #include <z3++.h>
// #include <unordered_map>
// #include <string>
// #include <iostream>

// using vara::solver::Z3Solver;

// std::unordered_map<Feature*, int>
// count_valid_configs_from_featureModel(const std::string& xmlPath, FeatureModel* model) {
//     std::unordered_map<Feature*, int> featureCounts;

//     std::unordered_map<std::string, Feature*> nameToFeature;
//     for (Feature* f : model->getAllFeatures()) {
//         nameToFeature[f->getName()] = f;
//     }

//     Z3Solver solver;
//     addXmlConstraintsToSolver(solver, xmlPath);
//     z3::solver& z3s = solver.getRawSolver();
//     const auto& allVars = solver.getAllVariables();

//     while (z3s.check() == z3::sat) {
//         z3::model m = z3s.get_model();
//         z3::expr_vector blocking(z3s.ctx());

//         for (const auto& [name, expr] : allVars) {
//             z3::expr val = m.eval(expr, true);

//             bool isTrue = val.is_bool() && val.bool_value() == Z3_L_TRUE;
//             if (isTrue && nameToFeature.count(name)) {
//                 Feature* f = nameToFeature.at(name);
//                 featureCounts[f]++;
//             }

//             if (val.is_bool()) {
//                 blocking.push_back(isTrue ? !expr : expr);
//             } else if (val.is_numeral()) {
//                 blocking.push_back(expr != val);
//             }
//         }

//         z3s.add(z3::mk_or(blocking));
//     }

//     return featureCounts;
// }

// std::unordered_map<Feature*, int>
// count_valid_configs_from_xml(const std::string& xmlPath) {
//     FeatureModel* model = FeatureDiagram::parseFeatureModelFromXML(xmlPath);
//     return count_valid_configs_from_featureModel(xmlPath, model);
// }


// int main(int argc, char** argv) {
//     if (argc != 2) {
//         std::cerr << "Verwendung: " << argv[0] << " <Pfad_zur_XML_Datei>" << std::endl;
//         return 1;
//     }

//     std::unordered_map<Feature*, int> featureCounts =
//         count_valid_configs_from_xml(argv[1]);

//     int total = 0;
//     for (const auto& [f, n] : featureCounts)
//         total += n;

//     std::cout << "\nGesamtanzahl Feature-Aktivierungen über alle gültigen Konfigurationen: " << total << "\n\n";
//     for (const auto& [f, n] : featureCounts) {
//         double percent = (100.0 * n) / total;
//         std::cout << "Feature \"" << f->getName() << "\": " << n << "x (" << percent << "%)" << std::endl;
//     }

//     return 0;
// }