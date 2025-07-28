#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "BDD/include/BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "BDD/include/Constraints.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;
using oxidd::capi::BDDFactory::BDDFeat;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {

    Result<SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        BDDFactory::BDDFeat* feat,
        size_t nodeCount,
        oxidd_bdd_t *oneTerminal,
        oxidd_bdd_t *zeroTerminal,
        unordered_map<std::string, BDDFactory::BDDFeat>* varMap
    ) {
        feat->info.marked = true;
        if (node->_p == zeroTerminal->_p && node->_i == zeroTerminal->_i) {
            feat->info.satCount = 0;
            feat->info.probability = -1.0
            return SolverErrorCode::Success;
        } else if (node->_p == oneTerminal->_p && node->_i == oneTerminal->_i) {
            feat->info.satCount = 1;
            feat->info.probability = -1.0;
            return SolverErrorCode::Success;
        } else {
            oxidd_level_no_t index_node = oxidd_bdd_level_no(*node);
            oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(*node);
            BDDFeat* trueFeat = findFeatureinBDD(&cofactors.first, varMap);
            oxidd_level_no_t index_high = oxidd_bdd_level_no(cofactors.first);
            BDDFeat* falseFeat = findFeatureinBDD(&cofactors.second, varMap);
            oxidd_level_no_t index_low = oxidd_bdd_level_no(cofactors.second);

            if(trueFeat->info.marked != feat->info.marked) {
                getPr(
                    manager,
                    &cofactors.first, 
                    trueFeat, 
                    nodeCount-1, 
                    oneTerminal, 
                    zeroTerminal, 
                    varMap
                );
            } else if(falseFeat->info.marked != feat->info.marked) {
                getPr(
                    manager, 
                    &cofactors.second, 
                    falseFeat, 
                    nodeCount-1, 
                    oneTerminal, 
                    zeroTerminal, 
                    varMap
                );
            }

            double solLow = falseFeat->info.satCount * std::pow(2, index_low - (index_node-1));
            double solHigh = trueFeat->info.satCount * std::pow(2, index_high - (index_node-1));
            feat->info.satCount = solLow + solHigh;
            feat->info.probability = solHigh / feat->info.satCount;
        }
    }
}