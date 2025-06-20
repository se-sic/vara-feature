#ifndef Z3_HELPER_HPP
#define Z3_HELPER_HPP

#include "Z3Solver.h"
#include <z3++.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "tinyxml2.h" 

std::vector<z3::expr> parseConstraints(z3::context &ctx, const std::vector<std::string> &constraints);

void addXmlConstraintsToSolver(Z3Solver &solver, const std::string &xmlPath);

#endif // Z3_HELPER_HPP