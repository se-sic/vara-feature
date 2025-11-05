#ifndef OXIDD_BDD_FACTORY
#define OXIDD_BDD_FACTORY

#include "oxidd/util.hpp"
#include "oxidd/bdd.hpp"
#include "vara/Feature/FeatureModel.h"
#include <string>
#include <unordered_map>

namespace bdd::sample
{
    class BDDFactory
    {
        public:
            struct BDDFeat {
                oxidd::bdd_function BddNode;                   // BDD node representing the feature
                bool IsRoot = false;                           // Is it the root feature?
                std::string Name;                              // Feature name
                bool Marked = false;                           // Marking for probability calculation
                size_t SatCount = 0;                           // Number of satisfying assignments
                std::optional<double> Probability;             // Probability of the node
            };

            enum BranchType : std::uint8_t { // Changed to use std::uint8_t as the base type
                TRUE,
                FALSE,
                NONE
            };

            BDDFeat* findFeatureinBDD(
                oxidd::bdd_function* Node,
                BranchType &BranchType
            );

            oxidd::bdd_function modelToBdd(const vara::feature::FeatureModel &Model);
            void fillManager(const vara::feature::FeatureModel &Model);

            std::unordered_map<oxidd::level_no_t, BDDFeat> VarMap;
            oxidd::bdd_manager Manager = oxidd::bdd_manager(1024L * 1024, 1024L * 1024, 8);
            oxidd::bdd_function FinalBdd = Manager.t();

    };
    
} // namespace bdd::sample

#endif // OXIDD_BDD_FACTORY