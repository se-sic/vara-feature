#include "ConfigCounter.hpp"
#include "Z3Helper.hpp"

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

int count_valid_configs_from_xml(const std::string &xmlPath,
                                 std::unordered_map<std::string, int> &featureCount) {
    Z3Solver solver;
    addXmlConstraintsToSolver(solver, xmlPath);
    z3::solver &z3s = solver.getRawSolver();
    std::map<std::string, z3::expr> allVars = solver.getAllVariables();

    int count = 0;

    while (z3s.check() == z3::sat) {
        count++;
        z3::model m = z3s.get_model();
        z3::expr_vector blockingClause(z3s.ctx());

        for (const auto &[name, expr] : allVars) {
            z3::expr val = m.eval(expr, true);

            if (val.is_bool()) {
                if (val.bool_value() == Z3_L_TRUE) {
                    featureCount[name]++;
                    blockingClause.push_back(!expr);
                } else {
                    blockingClause.push_back(expr);
                }
            } else if (val.is_numeral()) {
                blockingClause.push_back(expr != val);
            }
        }

        z3s.add(z3::mk_or(blockingClause));
    }

    return count;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Verwendung: " << argv[0] << " <Pfad_zur_XML_Datei>" << std::endl;
        return 1;
    }

    std::string xmlPath = argv[1];
    std::unordered_map<std::string, int> featureUsage;
    int totalConfigs = count_valid_configs_from_xml(xmlPath, featureUsage);

    if (totalConfigs == 0) {
        std::cout << "Keine gültige Konfiguration gefunden." << std::endl;
        return 0;
    }

    std::cout << "\nGesamtanzahl gültiger Konfigurationen: " << totalConfigs << "\n\n";
    std::cout << "Feature-Nutzung (Anteil über gültige Konfigurationen):\n";

    for (const auto &[feature, used] : featureUsage) {
        double percent = (100.0 * used) / totalConfigs;
        std::cout << "  " << feature << ": " << used << "x (" << percent << "%)" << std::endl;
    }

    return 0;
}

