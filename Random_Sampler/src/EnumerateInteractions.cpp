#include "EnumerateInteractions.h"
#include "oxidd/bdd.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

namespace bdd::sample {

    // Currently unused due to performance reasons, see header doc fir further information
    /*  
        oxidd::bdd_function Literal(const oxidd::bdd_manager &Manager, oxidd::var_no_t VarID, bool VarValue) { //NOLINT
            oxidd::bdd_function Int = Manager.var(VarID);
            return VarValue ? Int : ~Int;
        }
    */

    bool AreLiteralsSat(const std::vector<oxidd::var_no_t> &Variables, const std::vector<bool> &VarValues, const std::vector<std::vector<bool>> &ValidConfigs) {  //NOLINT Invalid case style for function 'AreLiteralsSat'
        for (const auto &Config : ValidConfigs) {
            bool Check = true;
            for (std::size_t I = 0; I < Variables.size(); ++I) {
                if (Config[Variables[I]] != VarValues[I]) {
                    Check = false;
                    break;
                }
            }
            if (Check) { 
                return true; 
            }
        }
        
        /*
            auto CopyBDD = FinalBDD;
            for (std::size_t I = 0; I < Variables.size(); ++I) {
                CopyBDD &= Literal(Manager, Variables[I], VarValues[I]);
                if (CopyBDD.is_invalid()) { 
                    throw std::runtime_error("BDD manager exhausted — increase inner_node_capacity");

                }
            }
            return CopyBDD.satisfiable();
        */
        
        return false;
    }

    std::vector<bdd::sample::Interaction> EnumerateInteractions(const oxidd::bdd_manager &Manager, unsigned T, const std::vector<std::vector<bool>> &ValidConfigs) { //NOLINT Invalid case style for function 'EnumerateInteractions'
        oxidd::var_no_t NumVars =  Manager.num_vars();
        std::vector<bdd::sample::Interaction> Outputs;
        std::vector<oxidd::var_no_t> Variables;
        Variables.reserve(T);

        // Subset selection: we pick  T-many variables in ascending order, then for each for each of the 2^T possible combinations we test satisfiability
        auto SelectVars =  [&](auto &&Self, oxidd::var_no_t Start, unsigned IntDepth) {
            if(T == IntDepth) {
                for (unsigned B = 0; B < (1U << T); B++) {
                    std::vector<bool> Values(T);
                    for(unsigned I = 0; I < T; I++) {
                        Values[I] = (B >> I) & 1U;
                    }
                    if(AreLiteralsSat(Variables, Values, ValidConfigs)) {
                        Outputs.push_back({.Variables=Variables, .Values=Values});
                    }
                }
                return;
            }
            for(oxidd::var_no_t VID = Start; VID <= NumVars- (T- IntDepth); VID++) {
                Variables.push_back(VID);
                Self(Self, VID +1, IntDepth + 1);
                Variables.pop_back();
            }
        };

        SelectVars(SelectVars,0, 0);
        std::cerr << T << "-wise interactions counted " << Outputs.size() << "\n";
        return Outputs;
    }
} //namespace bdd::sample
