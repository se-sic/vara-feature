#include "BDDFactory.h"
#include "Probabilities.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"
#include <oxidd/bdd.hpp>
#include <oxidd/bridge.hpp>
#include <oxidd/util.hpp>

using vara::Result;
using vara::solver::SolverErrorCode;
using bdd::sample::BDDFactory;

namespace bdd::sample {

    Result<vara::solver::SolverErrorCode>getPr( //NOLINT 
        const oxidd::bdd_manager &Manager,
        oxidd::bdd_function &Node,
        oxidd::var_no_t Id,
        BDDFactory::BDDFeat &Feat,
        BDDFactory &Factory
    ) {
        // Get the number of satisfying assignments for the BDD node and the total number of features
        BDDFactory::BranchType TB = BDDFactory::BranchType::TRUE;
        BDDFactory::BranchType FB = BDDFactory::BranchType::FALSE;
        double SatCount = Node.sat_count_double(Manager.num_vars());
        double TotalCount = std::pow(2, Manager.num_vars());
        oxidd::bdd_function CofactorTrue = Node.cofactor_true();
        oxidd::bdd_function CofactorFalse = Node.cofactor_false();
        // Using Bryant's algorithm to calculate the probabilities
        Feat.Marked = true;
        // Base cases: Terminals
        if(SatCount == 0 || SatCount == TotalCount) {
            return vara::Ok<void>();
        }
        // Recursive case until base case reached
        auto Opt = Manager.name_to_var(Feat.Name);
        if(!Opt.has_value()) {
            return SolverErrorCode::ILLEGAL_STATE;
        }
        oxidd::level_no_t IndexNode = Opt.value();
        BDDFactory::BDDFeat* TrueFeat = Factory.findFeatureinBDD(&CofactorTrue, TB);
        BDDFactory::BDDFeat* FalseFeat = Factory.findFeatureinBDD(&CofactorFalse, FB);
        oxidd::var_no_t IndexHigh;
        auto OptHigh = Manager.name_to_var(TrueFeat->Name);
        if(!OptHigh.has_value()) {
            if(TrueFeat->Name != "TRUE_TERMINAL") {
                return SolverErrorCode::ILLEGAL_STATE;
            }
            IndexHigh = Factory.getMaxLevel();
        } else {
            IndexHigh = OptHigh.value();
        }
        auto OptLow = Manager.name_to_var(FalseFeat->Name);
        oxidd::var_no_t IndexLow;
        if(!OptLow.has_value()) {
            if(FalseFeat->Name != "FALSE_TERMINAL") {
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
        } if(!FalseFeat->Marked) {
            getPr(
                Manager, 
                CofactorFalse,
                IndexLow, 
                *FalseFeat, 
                Factory
            );
        }

        double SolLow = CofactorFalse.sat_count_double(Manager.num_vars());//static_cast<double>(FalseFeat->SatCount) * std::pow(2.0, static_cast<double>(IndexLow) - (static_cast<double>(IndexNode) - 1.0)); //ALTERNATIVE: oxidd_bdd_sat_count_double(cofactors.second, oxidd_bdd_manager_num_vars(manager));
        double SolHigh = CofactorTrue.sat_count_double(Manager.num_vars());//static_cast<double>(TrueFeat->SatCount) * std::pow(2.0, static_cast<double>(IndexHigh) - (static_cast<double>(IndexNode) - 1.0)); //ALTERNATIVE: oxidd_bdd_sat_count_double(cofactors.first, oxidd_bdd_manager_num_vars(manager));
        Feat.SatCount = static_cast<size_t>(SolLow + SolHigh); //ALTERNATIVE: sat_count
        Feat.Probability = SolHigh / static_cast<double>(Feat.SatCount);
        return vara::Ok<void>();
    


//-------------------------------------------------------- OLD CODE --------------------------------------------------------
//      if (node->_p == zeroTerminal->_p && node->_i == zeroTerminal->_i) {
//          feat->info.satCount = 0;
//          feat->info.probability = -1.0;
//          return vara::Ok<void>();
//      } else if (node->_p == oneTerminal->_p && node->_i == oneTerminal->_i) {
//          feat->info.satCount = 1;
//          feat->info.probability = -1.0;
//          return vara::Ok<void>();
//      } else {
//             oxidd_level_no_t index_node = oxidd_bdd_level(*node);
//             oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(*node);
//             oxidd::capi::BDDFactory::BDDFeat* trueFeat = factory.findFeatureinBDD(&cofactors.first);
//             oxidd_level_no_t index_high = oxidd_bdd_level(cofactors.first);
//             oxidd::capi::BDDFactory::BDDFeat* falseFeat = factory.findFeatureinBDD(&cofactors.second);
//             oxidd_level_no_t index_low = oxidd_bdd_level(cofactors.second);

//             if(trueFeat->info.marked != feat->info.marked) {
//                 getPr(
//                     manager,
//                     &cofactors.first, 
//                     trueFeat, 
//                     nodeCount-1, 
//                     oneTerminal, 
//                     zeroTerminal, 
//                     factory
//                 );
//             } else if(falseFeat->info.marked != feat->info.marked) {
//                 getPr(
//                     manager, 
//                     &cofactors.second, 
//                     falseFeat, 
//                     nodeCount-1, 
//                     oneTerminal, 
//                     zeroTerminal, 
//                     factory
//                 );
//             }

//             double solLow = falseFeat->info.satCount * std::pow(2, index_low - (index_node-1));
//             double solHigh = trueFeat->info.satCount * std::pow(2, index_high - (index_node-1));
//             feat->info.satCount = solLow + solHigh;
//             feat->info.probability = solHigh / feat->info.satCount;
//             return vara::Ok<void>();
//         }
    }
} // namespace bdd::sample