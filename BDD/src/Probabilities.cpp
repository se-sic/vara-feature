#include "oxidd/capi.h"
#include <string>
#include <unordered_map>
#include "BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "Probabilities.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using oxidd::capi::BDDFactory;

namespace oxidd::capi {

    Result<vara::solver::SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        oxidd_var_no_t id,
        oxidd::capi::BDDFactory::BDDFeat* feat,
        oxidd::capi::BDDFactory& factory
    ) {
        // Get the number of satisfying assignments for the BDD node and the total number of features
        double sat_count = oxidd_bdd_sat_count_double(*node, oxidd_bdd_manager_num_vars(*manager));
        double total_count = std::pow(2, oxidd_bdd_manager_num_vars(*manager));
        // Using Bryant's algorithm to calculate the probabilities
        feat->marked = true;
        // Base cases: Terminals
        if(sat_count == 0.0){
            feat->satCount = 0;
            return vara::Ok<void>();
        } else if(sat_count == total_count) {
            feat->satCount = 1;
            return vara::Ok<void>();
        } else {
            // Recursive case until base case reached
            oxidd_level_no_t index_node = oxidd_bdd_manager_name_to_var(*manager, feat->name.c_str());
            oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(*node);
            oxidd::capi::BDDFactory::BDDFeat* trueFeat = factory.findFeatureinBDD(&cofactors.first);
            oxidd_level_no_t index_high = oxidd_bdd_manager_name_to_var(*manager, trueFeat->name.c_str());
            oxidd::capi::BDDFactory::BDDFeat* falseFeat = factory.findFeatureinBDD(&cofactors.second);
            oxidd_level_no_t index_low = oxidd_bdd_manager_name_to_var(*manager, falseFeat->name.c_str());

            if(trueFeat->marked != feat->marked) {
                getPr(
                    manager,
                    &cofactors.first, 
                    index_high, 
                    trueFeat, 
                    factory
                );
            } else if(falseFeat->marked != feat->marked) {
                getPr(
                    manager, 
                    &cofactors.second, 
                    index_low, 
                    falseFeat, 
                    factory
                );
            }

            double solLow =  falseFeat->satCount * std::pow(2, index_low - (index_node-1)); //ALTERNATIVE: oxidd_bdd_sat_count_double(cofactors.second, oxidd_bdd_manager_num_vars(manager));
            double solHigh = trueFeat->satCount * std::pow(2, index_high - (index_node-1)); //ALTERNATIVE: oxidd_bdd_sat_count_double(cofactors.first, oxidd_bdd_manager_num_vars(manager));
            feat->satCount = solLow + solHigh; //ALTERNATIVE: sat_count
            feat->probability = solHigh / feat->satCount;
            return vara::Ok<void>();
        } 



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
}