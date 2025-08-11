#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Feature.h"


namespace oxidd::capi
{
    class BDDFactory
    {
        public:
            enum class featType {
            NUMERIC,
            BINARY
            };

            struct nodeInfo {
                bool marked = false;
                size_t satCount = 0; // Number of satisfying assignments
                double probability = 0.0; // Probability of the node
            };

            struct BDDFeat {
            featType type;
            std::variant<oxidd_bdd_t*, std::vector<std::pair<string, oxidd_bdd_t>>*> data;
            nodeInfo info;
            bool isRoot = false;
            };

            std::unordered_map<std::string, BDDFeat> varMap;

            BDDFeat* findFeatureinBDD(
                oxidd_bdd_t* node
            );

            oxidd_bdd_t modelToBdd(const vara::feature::FeatureModel &model);
    };
    
} // namespace oxidd::capi

#endif // OXIDD_BDD_FACTORY