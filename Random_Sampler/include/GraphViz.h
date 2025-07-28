#ifndef GRAPHVIZ_BDD_H
#define GRAPHVIZ_BDD_H

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include <iostream>
#include <string>
#include <vector>
#include "../../BDD/include/BDDFactory.h"

using oxidd::bdd_manager;
using oxidd::bdd_function;
using oxidd::capi::oxidd_bdd_manager_t;
using oxidd::capi::oxidd_bdd_t;
using std::string;
using std::vector;
using std::pair;

int visBDD(const oxidd_bdd_manager_t &manager, const oxidd_bdd_t &f, 
           const std::unordered_map<string, oxidd::capi::BDDFactory::BDDFeat> &vars,
           std::string &funcname, int num_func, 
           const std::string &filepath = "../results/bdd.dot");

#endif // GRAPHVIZ_BDD_H