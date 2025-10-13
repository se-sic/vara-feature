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
using oxidd::capi::oxidd::bdd_manager;
using oxidd::capi::oxidd::bdd_function;
using std::string;
using std::vector;
using std::pair;

int visBDD(const oxidd::bdd_manager &manager, const oxidd::bdd_function &f, 
           const std::unordered_map<string, oxidd::capi::BDDFactory::BDDFeat> &vars,
           std::string &funcname, int num_func, 
           const std::string &filepath = "../results/bdd.dot");

#endif // GRAPHVIZ_BDD_H