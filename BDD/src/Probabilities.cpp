#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "BDDFactory.h"
#include "llvm/Support/ErrorHandling.h"
#include "vara/Utils/Result.h"
#include "vara/Solver/Error.h"
#include "Constraints.h"

using vara::Result;
using vara::solver::SolverErrorCode;
using vara::feature::Feature;
using oxidd::capi::BDDFactory;
using std::unordered_map;
using std::string;
using std::vector;
using std::pair;

namespace oxidd::capi {

    Result<vara::solver::SolverErrorCode>getPr(
        oxidd_bdd_manager_t* manager,
        oxidd_bdd_t* node,
        oxidd::capi::BDDFactory::BDDFeat* feat,
        size_t nodeCount,
        oxidd_bdd_t *oneTerminal,
        oxidd_bdd_t *zeroTerminal,
        BDDFactory& factory
    ) {
        feat->info.marked = true;
        if (node->_p == zeroTerminal->_p && node->_i == zeroTerminal->_i) {
            feat->info.satCount = 0;
            feat->info.probability = -1.0;
            return vara::Ok<void>();
        } else if (node->_p == oneTerminal->_p && node->_i == oneTerminal->_i) {
            feat->info.satCount = 1;
            feat->info.probability = -1.0;
            return vara::Ok<void>();
        } else {
            oxidd_level_no_t index_node = oxidd_bdd_level(*node);
            oxidd_bdd_pair_t cofactors = oxidd_bdd_cofactors(*node);
            oxidd::capi::BDDFactory::BDDFeat* trueFeat = factory.findFeatureinBDD(&cofactors.first);
            oxidd_level_no_t index_high = oxidd_bdd_level(cofactors.first);
            oxidd::capi::BDDFactory::BDDFeat* falseFeat = factory.findFeatureinBDD(&cofactors.second);
            oxidd_level_no_t index_low = oxidd_bdd_level(cofactors.second);

            if(trueFeat->info.marked != feat->info.marked) {
                getPr(
                    manager,
                    &cofactors.first, 
                    trueFeat, 
                    nodeCount-1, 
                    oneTerminal, 
                    zeroTerminal, 
                    factory
                );
            } else if(falseFeat->info.marked != feat->info.marked) {
                getPr(
                    manager, 
                    &cofactors.second, 
                    falseFeat, 
                    nodeCount-1, 
                    oneTerminal, 
                    zeroTerminal, 
                    factory
                );
            }

            double solLow = falseFeat->info.satCount * std::pow(2, index_low - (index_node-1));
            double solHigh = trueFeat->info.satCount * std::pow(2, index_high - (index_node-1));
            feat->info.satCount = solLow + solHigh;
            feat->info.probability = solHigh / feat->info.satCount;
        }
    }
}