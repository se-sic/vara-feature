// #include "GraphViz.h"
// #include "../../BDD/include/BDDFactory.h"

// using oxidd::bdd_manager;
// using oxidd::bdd_function;
// using oxidd::capi::oxidd::bdd_manager;
// using oxidd::capi::oxidd::bdd_function;
// using std::string;
// using std::vector;
// using std::pair;


// int visBDD(const oxidd::bdd_manager* manager, const oxidd::bdd_function* f, const std::unordered_map<string, oxidd::capi::BDDFactory::BDDFeat> &vars,
//            string &funcname, int num_func, const string &filepath) {

//     const oxidd::bdd_function functions[] = { *f };
//     const char *function_names[] = { funcname.c_str() };

//     vector<oxidd::bdd_function> var_bdds;
//     vector<const char*> var_names;
//     vector<string> var_names_str;
//     oxidd::bdd_function featureVar;

//     for(const auto &pair: vars) {
//         const auto &name = pair.first;
//         const auto &feat = pair.second;
//         if (feat.type == oxidd::capi::BDDFactory::featType::NUMERIC) {
//             auto numericFeats = std::get<std::vector<std::pair<string, oxidd::bdd_function>>*>(feat.data);
//             featureVar = std::find_if(numericFeats->begin(), numericFeats->end(), [&name](const std::pair<std::string, oxidd::bdd_function>& pair){
//                         return pair.first == name;}
//                         )->second;
//         } else if (feat.type == oxidd::capi::BDDFactory::featType::BINARY) {
//             auto* bddPointer = std::get<oxidd::capi::oxidd::bdd_function*>(feat.data);
//             featureVar = *bddPointer;
//         } else {
//             return 1;
//         }
//         var_bdds.push_back(*reinterpret_cast<const oxidd::bdd_function*>(&featureVar));
//         var_names_str.push_back(name);
//     }

//     for(const auto &n: var_names_str) {
//         var_names.push_back(n.c_str());
//     }

//     bool check = oxidd_bdd_manager_dump_all_dot_file(
//         *manager,
//         filepath.c_str(),
//         functions,
//         function_names,
//         1,
//         var_bdds.data(),
//         var_names.data(),
//         var_bdds.size()
//     );

//     if(!check) {
//         std::cerr << "Error: Could not write to file." << std::endl;
//         return 1;
//     } else {
//         std::cout << "Graph written to ../results/bdd.dot" << std::endl;
//         std::cout << "Run:\n  dot -Tpng ../results/bdd.dot -o ../results-BDDs/bdd.png\n";
//     }
// }