#include "Z3Solver.h"
#include <z3++.h>
#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> generate_fd_constraints(const std::vector<std::string> &vars) {
    std::vector<std::string> constraints;
    for (const auto &var : vars) {
        constraints.push_back("(assert (> " + var + " 0))");
    }
    return constraints;
}

int count_valid_configs(const std::vector<std::string> &vars) {
    Z3Solver solver;
    z3::context &ctx = solver.getContext();

    auto constraints = generate_fd_constraints(vars);
    for (const auto &c : constraints) {
        z3::expr e = ctx.parse_string(c.c_str());
        solver.addConstraint(e);
    }

    return solver.check() ? 1 : 0;
}

int count_valid_configs_from_xml(const std::string &xmlPath) {
    Z3Solver solver;
    addXmlConstraintsToSolver(solver, xmlPath);

    int count = 0;
    z3::solver &z3s = solver.getRawSolver();
    std::vector<z3::expr> trackedVars;

    for (auto &v : solver.getAllVariables()) {
        trackedVars.push_back(v.second);
    }

    while (z3s.check() == z3::sat) {
        count++;
        z3::model m = z3s.get_model();
        z3::expr_vector block(z3s.ctx());
        for (auto &v : trackedVars) {
            z3::expr val = m.eval(v, true);
            block.push_back(val.bool_value() == Z3_L_TRUE ? !v : v);
        }
        z3s.add(z3::mk_or(block));
    }

    return count;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Verwendung: " << argv[0] << " <Pfad_zur_XML_Datei>" << std::endl;
        return 1;
    }

    std::string xmlPath = argv[1];
    int totalConfigs = count_valid_configs_from_xml(xmlPath);

    std::cout << "Gültige Konfigurationen: " << totalConfigs << std::endl;
    return 0;
}
