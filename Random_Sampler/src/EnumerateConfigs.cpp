#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <stdexcept>
#include <vector>

namespace bdd::sample {

    std::vector<std::vector<bool>> EnumerateConfigs(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &Root) { //NOLINT
        //Number of variables in current BDD
        oxidd::var_no_t NumVars =  Manager.num_vars();
        //All finished configurations
        std::vector<std::vector<bool>> Outputs;
        //Buffer of lenght of single configuration 
        std::vector<bool> PartialConfig(NumVars);

        //If the current node is a terminal, we push the current configuration from the Buffer
        //Else we have to consider all remaining variables with all possible values (i.e. True and False) until Terminal reached
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

        auto CreatePartialConfig = [&](auto &&Self, const oxidd::bdd_function &Node, oxidd::var_no_t CurrentVarId) {
            if (Node.is_invalid()){
                throw std::logic_error("Invalid BDD node encountered during enumeration.");
            }
            //Terminal Check: If True-Terminal it is satisfiable, else False-Terminal adn we return
            auto NodeOpt = Node.node_var();
            if (!NodeOpt.has_value()) {
                if(Node.satisfiable()) {
                    //If True-Terminal, we can consider the current PartialConfig
                    EmitConfigs(EmitConfigs, CurrentVarId);
                }
                return;
            }                  
            oxidd::var_no_t NodeVarId = NodeOpt.value();

            //BDD-Traversal: First block only considers skipped variables and assigns them both possible values
            //Second block traverses BDD following all the edges to the Terminal
            if(CurrentVarId < NodeVarId) {
                PartialConfig[CurrentVarId] = false;
                Self(Self, Node, CurrentVarId+1);
                PartialConfig[CurrentVarId] = true;
                Self(Self, Node, CurrentVarId+1);
            } else if(CurrentVarId == NodeVarId) {
                PartialConfig[NodeVarId] = false;
                Self(Self, Node.cofactor_false(), NodeVarId+1);
                PartialConfig[NodeVarId] = true;
                Self(Self, Node.cofactor_true(), NodeVarId+1);
            } else {
                throw std::logic_error("Unexpected BDD structure during enumeration.");
            }           
        };

        CreatePartialConfig(CreatePartialConfig, Root, 0);
        return Outputs;
    }
} //namespace bdd::sample