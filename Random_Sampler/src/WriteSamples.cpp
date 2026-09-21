#include "WriteSamples.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace bdd::sample {

    void WriteSampleCSV(const oxidd::bdd_manager &Manager, const std::vector<std::vector<bool>> &Sample, std::string &FilePath) { //NOLINT
        std::ofstream Out(FilePath);
        if (!Out) {
            std::cerr << "Error: Could not open file for writing" << FilePath << "\n";
            return;
        }

        // Header row: Skip root feature (Variable 0)
        for (oxidd::var_no_t V = 1; V < Manager.num_vars(); ++V) {
            const std::string VName = Manager.var_name(V);
            assert(!VName.empty() && "Variable name should exist");
            if (V == 1) { 
                Out << VName; 
                continue; 
            }
            Out << "," << VName;
        }
        Out << "\n";
        
        // Data rows: Also skip root feature (Variable 0)
        for (const auto &Config : Sample) {
            for (oxidd::var_no_t V = 1; V < Config.size(); ++V) {
                if (V == 1) { 
                    Out << (Config[V] ? "1" : "0"); 
                    continue; 
                }
                Out << "," << (Config[V] ? "1" : "0");
            }
            Out << "\n";
        }
        Out.close();
    }

} //namespace bdd::sample