#include "GraphViz.hpp"

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;
using std::string;
using std::vector;
using std::pair;


int visBDD(const bdd_manager &manager, const bdd_function &f, const vector<pair<bdd_function, string>> &vars, 
           const string &filepath = "../results/bdd.dot", string &funcname, int num_func) {

    const oxidd_bdd_manager_t* ptr_manager = reinterpret_cast<const oxidd_bdd_manager_t*>(&manager);
    const oxidd_bdd_t* ptr_f = reinterpret_cast<const oxidd_bdd_t*>(&f);
    const oxidd_bdd_t functions[] = { *ptr_f };
    const char *function_names[] = { funcname.c_str() };

    vector<oxidd_bdd_t> var_bdds;
    vector<const char*> var_names;
    vector<string> var_names_str;

    for(const auto &[bdd, name]: vars) {
        var_bdds.push_back(*reinterpret_cast<const oxidd_bdd_t*>(&bdd));
        var_names_str.push_back(name);
    }

    for(const auto &n: var_names_str) {
        var_names.push_back(n.c_str());
    }

    bool check = oxidd_bdd_manager_dump_all_dot_file(
        *ptr_manager,
        filepath.c_str(),
        functions,
        function_names,
        1,
        var_bdds.data(),
        var_names.data(),
        var_bdds.size()
    );

    if(!check) {
        std::cerr << "Error: Could not write to file." << std::endl;
        return 1;
    } else {
        std::cout << "Graph written to ../results/bdd.dot" << std::endl;
        std::cout << "Run:\n  dot -Tpng ../results/bdd.dot -o ../results-BDDs/bdd.png\n";
    }
}