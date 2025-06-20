#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include <iostream>

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;

int visBDD() {
    bdd_manager manager(32,320,1);

    bdd_function a = manager.new_var();
    bdd_function b = manager.new_var();
    bdd_function c = manager.new_var();
    bdd_function f = a & b | c;

    const oxidd_bdd_manager_t* ptr_manager = reinterpret_cast<const oxidd_bdd_manager_t*>(&manager);
    const oxidd_bdd_t* ptr_a = reinterpret_cast<const oxidd_bdd_t*>(&a);
    const oxidd_bdd_t* ptr_b = reinterpret_cast<const oxidd_bdd_t*>(&b);
    const oxidd_bdd_t* ptr_c = reinterpret_cast<const oxidd_bdd_t*>(&c);
    const oxidd_bdd_t* ptr_f = reinterpret_cast<const oxidd_bdd_t*>(&f);

    const oxidd_bbd_t functions[] = { *ptr_f };
    const char* names[] = { "f" };

    const oxidd_bdd_t vars[] = { *ptr_a, *ptr_b, *ptr_c };
    const char* var_names[] = { "a", "b", "c" };

    bool check = oxidd_bdd_manager_dump_all_dot_file(
        *ptr_manager,
        "../results/bdd.dot",
        functions,
        names,
        1,
        vars,
        var_names,
        3
    );

    if(!check) {
        std::cerr << "Error: Could not write to file." << std::endl;
        return 1;
    } else {
        std::cout << "Graph written to ../results/bdd.dot" << std::endl;
        std::cout << "Run:\n  dot -Tpng ../results/bdd.dot -o ../results-BDDs/bdd.png\n";
    }
}