#include "BDDFactory.h"
#include "Probabilities.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"
#include <cstddef>
#include <oxidd/bdd.hpp>
#include <oxidd/bridge.hpp>
#include <oxidd/util.hpp>

using vara::Result;
using vara::solver::SolverErrorCode;
using bdd::sample::BDDFactory;

namespace bdd::sample {
    // Using Bryant's algorithm to calculate the probabilities used for uniform, ranodm samplin
    Result<vara::solver::SolverErrorCode>getPr( //NOLINT 
        const oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Node,
        oxidd::var_no_t Id,
        BDDFactory::BDDFeat &Feat,
        BDDFactory &Factory
    ) {
        Feat.BddNode = Node; //Update the BDD node to fit new constraints
        Feat.CC = Node.sat_count_double(Manager.num_vars()-Id);
        // double SatCount = Manager.num_vars();
        // double TotalCount = std::pow(2, Manager.num_vars());

        if(Feat.Marked) {
            return vara::Ok<void>();
        }

        Feat.Marked = true;

        // Base cases: Terminals
        //1-Terminal
        if(Feat.Name == "TRUE_TERMINAL") {
            return vara::Ok<void>();
        }
        //0-Terminal
        if(Feat.Name == "FALSE_TERMINAL") {
            return vara::Ok<void>();
        }

        Feat.SatCount = static_cast<size_t>(Node.sat_count_double(Manager.num_vars()-Id));
        oxidd::bdd_function CofactorTrue = Node.cofactor_true();
        oxidd::bdd_function CofactorFalse = Node.cofactor_false();

         // Recursive case: Non-terminal nodes

        BDDFactory::BDDFeat* TrueFeat = Factory.findFeatureinBDD(&CofactorTrue);
        BDDFactory::BDDFeat* FalseFeat = Factory.findFeatureinBDD(&CofactorFalse);

        oxidd::var_no_t IndexHigh;
        auto OptHigh = Manager.name_to_var(TrueFeat->Name);
        if(!OptHigh.has_value()) {
            if(TrueFeat->Name != "TRUE_TERMINAL" && TrueFeat->Name != "FALSE_TERMINAL") {
                return SolverErrorCode::ILLEGAL_STATE;
            }
            IndexHigh = Factory.getMaxLevel();
        } else {
            IndexHigh = OptHigh.value();
        }

        auto OptLow = Manager.name_to_var(FalseFeat->Name);
        oxidd::var_no_t IndexLow;
        if(!OptLow.has_value()) {
            if(FalseFeat->Name != "FALSE_TERMINAL" && FalseFeat->Name != "TRUE_TERMINAL") {
                return SolverErrorCode::ILLEGAL_STATE;
            }
            IndexLow = Factory.getMaxLevel();
        } else {
            IndexLow = OptLow.value();
        }

        if(!TrueFeat->Marked) {
            getPr(
                Manager, 
                CofactorTrue,
                IndexHigh, 
                *TrueFeat, 
                Factory
            );
        }

        if(!FalseFeat->Marked) {
            getPr(
                Manager,
                CofactorFalse,
                IndexLow, 
                *FalseFeat, 
                Factory
            );
        } 

        /* Alternative manual prob calculation NO OXIDD 
            auto GapLo= static_cast<double>(IndexLow - IndexNode -1);
            auto GapHi= static_cast<double>(IndexHigh - IndexNode -1);
            GapLo = std::max<double>(GapLo, 0);
            GapHi = std::max<double>(GapHi, 0);
            double SolLo = static_cast<double>(FalseFeat->SatCount) * std::pow(2.0, GapLo);
            double SolHi = static_cast<double>(TrueFeat->SatCount) * std::pow(2.0, GapHi);
            double SolN = SolLo + SolHi;
            Feat.SatCount = static_cast<size_t>(SolN);
            Feat.Probability = (SolN == 0) ? 0.0 : (SolHi / SolN);
        */

        auto SolLo = static_cast<double>(FalseFeat->SatCount); //NOLINT
        auto SolHi = static_cast<double>(TrueFeat->SatCount);
        auto SolN = static_cast<double>(Feat.SatCount);
        Feat.Probability = (SolN == 0) ? 0.0 : (SolHi / SolN);
        return vara::Ok<void>();
    }
} // namespace bdd::sample