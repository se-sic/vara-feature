#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include <iostream>

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;

int main() {

//Look at bdd.hpp in /oxidd/bindings/cpp/include/oxidd/bdd.hpp for documentation of the BDD API
//Initializes the BDD manager with 32 vars, 320 nodes, and 1 cache entry
bdd_manager mgr(32, 320, 1);

bdd_function a = mgr.new_var();
bdd_function b = mgr.new_var();
bdd_function c =  mgr.new_var();
bdd_function f = a & b | ~a & c | b & ~c;

//Look at bdd.rs in /oxidd/crates/oxidd-ffi-c/src/bdd.rs for documentation of the BDD API
//Recast the C++ BDD objects to C API types in order to use the C API function oxidd_bdd_manager_dump_all_dot_file
const oxidd_bdd_manager_t* c_mgr = reinterpret_cast<const oxidd_bdd_manager_t*>(&mgr);
const oxidd_bdd_t* c_a = reinterpret_cast<const oxidd_bdd_t*>(&a);
const oxidd_bdd_t* c_b = reinterpret_cast<const oxidd_bdd_t*>(&b);
const oxidd_bdd_t* c_c = reinterpret_cast<const oxidd_bdd_t*>(&c);
const oxidd_bdd_t* c_f = reinterpret_cast<const oxidd_bdd_t*>(&f);

const oxidd_bdd_t functions[] = { *c_f };
const char* function_names[] = { "Function" };

const oxidd_bdd_t vars[] = { *c_a, *c_b, *c_c };
const char* var_names[] = { "a", "b", "c"};

bool success = oxidd_bdd_manager_dump_all_dot_file(
    *c_mgr,
    "../../results-BDDs/bdd.dot",
    functions,
    function_names,
    1,
    vars,
    var_names,
    3
);

if (success) {
    std::cout << "DOT file created: bdd.dot\n";
    std::cout << "Run:\n  dot -Tpng ../../results-BDDs/bdd.dot -o ../../results-BDDs/bdd.png\n";
} else {
    std::cerr << "Failed to dump DOT file.\n";
}

return 0;
}