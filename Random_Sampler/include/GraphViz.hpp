#ifndef GRAPHVIZ_BDD_HPP
#define GRAPHVIZ_BDD_HPP

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include <iostream>
#include <string>
#include <vector>

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;
using std::string;
using std::vector;
using std::pair;

int visBDD(const oxidd::bdd_manager &manager, const oxidd::bdd_function &f, 
           const std::vector<pair<oxidd::bdd_function, std::string>> &vars,
           std::string &funcname, int num_func, 
           const std::string &filepath = "../results/bdd.dot");

#endif // GRAPHVIZ_BDD_HPP