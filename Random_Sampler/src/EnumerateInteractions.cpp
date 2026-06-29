#include "EnumerateInteractions.h"
#include "oxidd/bdd.hpp"
#include <vector>

namespace bdd::sample {

    oxidd::bdd_function Literal(const oxidd::bdd_manager &Manager, oxidd::var_no_t VarID, bool VarValue) { //NOLINT
        oxidd::bdd_function Int = Manager.var(VarID);
        return VarValue ? Int : ~Int;
    }

    bool AreLiteralsSat(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &FinalBDD, const std::vector<oxidd::var_no_t> &Variables, const std::vector<bool> &VarValues) { //NOLINT
        auto TestBDD = FinalBDD;
        for (std::size_t I = 0; I < Variables.size(); ++I) {
            TestBDD &= Literal(Manager, Variables[I], VarValues[I]);
        }
        return TestBDD.satisfiable();
    }

    std::vector<bdd::sample::Interaction> EnumerateInteractions(const oxidd::bdd_manager &Manager, const oxidd::bdd_function &FinalBDD, unsigned T) { //NOLINT
        //Number of variables in current BDD
        oxidd::var_no_t NumVars =  Manager.num_vars();
        //All valid interactions - what we return 
        std::vector<bdd::sample::Interaction> Outputs;
        //Currently considered Nodes 
        std::vector<oxidd::var_no_t> Variables;
        Variables.reserve(T);

        //
        auto SelectVars =  [&](auto &&Self, oxidd::var_no_t Start, unsigned IntDepth) {
            if(T == IntDepth) {
                for (unsigned B = 0; B < (1U << T); B++) {
                    std::vector<bool> Values(T);
                    for(unsigned I = 0; I < T; I++) {
                        Values[I] = (B >> I) & 1U;
                    }
                    if(AreLiteralsSat(Manager, FinalBDD, Variables, Values)) {
                        Outputs.push_back({Variables, Values});
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
        return Outputs;
    }
} //namespace bdd::sample
