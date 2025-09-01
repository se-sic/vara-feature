#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include <vector>
#include <string>
#include <unordered_map>
#include "vara/Feature/Feature.h"
#include "Probabilities.h"
#include "Constraint.h"

namespace oxidd::capi
{
    class BDDFactory
    {
        public:
            struct BDDFeat {
                oxidd_bdd_t bddNode;                   // BDD node representing the feature
                bool isRoot = false;                   // Is it the root feature?
                std::string name = "";                 // Feature name
                bool marked = false;                   // Marking for probability calculation
                size_t satCount = 0;                   // Number of satisfying assignments
                std::optional<double> probability = {} // Probability of the node
            };

            BDDFeat* findFeatureinBDD(
                oxidd_bdd_t* node
            );

            oxidd_bdd_t BDDFactory::modelToBdd(const vara::feature::FeatureModel &model);
            void BDDFactory::fillManager(const vara::feature::FeatureModel &model);

            std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat> varMap;
            oxidd_bdd_manager_t manager = oxidd_bdd_manager_new(0, 0, 0);
            oxidd_bdd_t finalBdd = oxidd_bdd_true(manager); 

    };
    
} // namespace oxidd::capi

#endif // OXIDD_BDD_FACTORY