#include "EnumerateConfigs.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"

#include <cassert>
#include <iostream>
#include <vector>

namespace bdd::sample {

    std::vector<std::vector<bool>> EnumerateConfigs(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &Root) { //NOLINT Invalid case style for function 'EnumerateConfigs'
        oxidd::var_no_t NumVars =  Manager.num_vars();
        std::vector<std::vector<bool>> Outputs;
        std::vector<bool> PartialConfig(NumVars);

        // When the BDD has reached a true terminal, but there are still unchecked variables remaining, we have to declare them as
        // unconstrained. This measn that we emit all 2^(remaining variables) possible combinations by recursion.
        auto EmitConfigs = [&](auto &&Self, oxidd::var_no_t NextVarId) {
            if(NextVarId == NumVars) {
                Outputs.push_back(PartialConfig);
                return;
            }
            PartialConfig[NextVarId] = false;
            Self(Self, NextVarId+1);
            PartialConfig[NextVarId] = true;
            Self(Self, NextVarId+1);
        };

        // Depth-First Search through the BDD: At each step, either we check the current variable (branching into cofactors) or
        // we skip it (variables becomes unconstrained -> both possible values have to emitted). A false terminal returns without
        // emitting anything.
        auto CreatePartialConfig = [&](auto &&Self, const oxidd::bdd_function &Node, oxidd::var_no_t CurrentVarId) {
            if (Node.is_invalid()) {
                std::cerr << "Error: Invalid BDD node during enumeration" << "\n";
                return;
            }

            //Terminal Check: If True-Terminal it is satisfiable and we emit the current path, else False-Terminal and we return
            auto NodeOpt = Node.node_var();
            if (!NodeOpt.has_value()) {
                if(Node.satisfiable()) {
                    EmitConfigs(EmitConfigs, CurrentVarId);
                }
                return;
            }                  
            oxidd::var_no_t NodeVarId = NodeOpt.value();

            if(CurrentVarId < NodeVarId) {
                // Skipped variable: BDD reduced it -> we have to try both values
                PartialConfig[CurrentVarId] = false;
                Self(Self, Node, CurrentVarId+1);
                PartialConfig[CurrentVarId] = true;
                Self(Self, Node, CurrentVarId+1);
            } else if(CurrentVarId == NodeVarId) {
                // Branching into cofactors
                PartialConfig[NodeVarId] = false;
                Self(Self, Node.cofactor_false(), NodeVarId+1);
                PartialConfig[NodeVarId] = true;
                Self(Self, Node.cofactor_true(), NodeVarId+1);
            } else {
                std::cerr << "Error: CurrentVarID > NodeVarID should never happen." << "\n";
                return;
            }           
        };

        CreatePartialConfig(CreatePartialConfig, Root, 0);
        return Outputs;
    }

} //namespace bdd::sample